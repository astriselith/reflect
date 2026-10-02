#include "clang/AST/AST.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclTemplate.h"
#include "clang/AST/TemplateBase.h"
#include "clang/AST/Type.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Lex/Pragma.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"
#include <cctype>
#include <fstream>
#include <iterator>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>

using namespace clang;
using namespace std;

namespace {

string sanitizePath(const string &path) {
  string out;
  out.reserve(path.size());
  for (char c : path) {
    if (isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '.') {
      out += c;
    } else {
      out += '_';
    }
  }
  return out;
}

string escapeForStringLiteral(const string &text) {
  string out;
  out.reserve(text.size());
  for (char c : text) {
    if (c == '\\' || c == '"') {
      out += '\\';
    }
    out += c;
  }
  return out;
}

const char *accessKeyword(AccessSpecifier access) {
  switch (access) {
  case AS_public:
    return "PUBLIC";
  case AS_protected:
    return "PROTECTED";
  case AS_private:
    return "PRIVATE";
  default:
    return "PUBLIC";
  }
}

string qualifiedTypeName(QualType qt) {
  while (qt->isReferenceType()) {
    qt = qt.getNonReferenceType();
  }
  while (qt->isPointerType()) {
    qt = qt->getPointeeType();
  }
  while (qt->isArrayType()) {
    qt = qt->getAsArrayTypeUnsafe()->getElementType();
  }
  qt = qt.getUnqualifiedType();

  if (const auto *typedefType = qt->getAs<TypedefType>()) {
    const TypedefNameDecl *decl = typedefType->getDecl();
    QualType underlying = decl->getUnderlyingType();
    if (const auto *record = underlying->getAsCXXRecordDecl()) {
      return record->getQualifiedNameAsString();
    }
    return decl->getQualifiedNameAsString();
  }

  if (const auto *record = qt->getAsCXXRecordDecl()) {
    return record->getQualifiedNameAsString();
  }

  if (const auto *enumType = qt->getAs<EnumType>()) {
    return enumType->getDecl()->getQualifiedNameAsString();
  }

  if (const auto *spec = qt->getAs<TemplateSpecializationType>()) {
    const TemplateDecl *templateDecl =
        spec->getTemplateName().getAsTemplateDecl();
    if (templateDecl) {
      string result = templateDecl->getQualifiedNameAsString() + "<";
      bool first = true;
      for (const TemplateArgument &arg : spec->template_arguments()) {
        if (!first) {
          result += ", ";
        }
        first = false;
        if (arg.getKind() == TemplateArgument::Type) {
          result += qualifiedTypeName(arg.getAsType());
        } else {
          result += "_";
        }
      }
      result += ">";
      return result;
    }
  }

  return qt.getAsString();
}

string namespaceOf(const CXXRecordDecl *record) {
  string namespaceName;
  for (const DeclContext *context = record->getDeclContext();
       context != nullptr && !context->isTranslationUnit();
       context = context->getParent()) {
    if (const auto *namespaceDecl = dyn_cast<NamespaceDecl>(context)) {
      namespaceName = namespaceDecl->getQualifiedNameAsString();
      break;
    }
  }
  return namespaceName;
}

string symbolSuffix(const string &qualifiedName) {
  string suffix;
  suffix.reserve(qualifiedName.size());
  for (size_t i = 0; i < qualifiedName.size(); ++i) {
    char c = qualifiedName[i];
    if (c == ':' && i + 1 < qualifiedName.size() && qualifiedName[i + 1] == ':') {
      suffix += '_';
      ++i;
    } else if (c == '<' || c == '>' || c == ',' || c == ' ' || c == '*') {
      suffix += '_';
    } else if (c == '&') {
      continue;
    } else {
      suffix += c;
    }
  }
  return suffix;
}

string absolutePath(const string &path) {
  if (llvm::sys::path::is_absolute(path)) {
    return path;
  }
  llvm::SmallString<256> abs(path);
  llvm::sys::fs::make_absolute(abs);
  return abs.str().str();
}

const CXXRecordDecl *findPolymorphicBase(const CXXRecordDecl *record) {
  set<const CXXRecordDecl *> visited;
  vector<const CXXRecordDecl *> work;
  for (const CXXBaseSpecifier &b : record->bases()) {
    const CXXRecordDecl *base = b.getType()->getAsCXXRecordDecl();
    if (base) {
      work.push_back(base);
    }
  }
  while (!work.empty()) {
    const CXXRecordDecl *current = work.back();
    work.pop_back();
    if (!visited.insert(current).second) {
      continue;
    }
    if (current->isPolymorphic()) {
      return current;
    }
    for (const CXXBaseSpecifier &b : current->bases()) {
      const CXXRecordDecl *base = b.getType()->getAsCXXRecordDecl();
      if (base) {
        work.push_back(base);
      }
    }
  }
  return nullptr;
}

class FileLock {
public:
  explicit FileLock(const string &path) : fd_(-1) {
    string lockPath = path + ".reflect.lock";
    fd_ = ::open(lockPath.c_str(), O_CREAT | O_RDWR, 0644);
    if (fd_ >= 0) {
      ::flock(fd_, LOCK_EX);
    }
  }
  ~FileLock() {
    if (fd_ >= 0) {
      ::flock(fd_, LOCK_UN);
      ::close(fd_);
    }
  }
  bool valid() const { return fd_ >= 0; }

private:
  int fd_;
};

bool injectFriend(const string &headerPath, const string &className,
                  const string &helperClass, unsigned braceOffset) {
  FileLock lock(headerPath);
  if (!lock.valid()) {
    return false;
  }

  ifstream in(headerPath);
  if (!in) {
    return false;
  }
  string content((istreambuf_iterator<char>(in)),
                 istreambuf_iterator<char>());
  in.close();

  string needle = "friend class " + helperClass;
  if (content.find(needle) != string::npos) {
    return false;
  }

  string insertion = "\n    friend class " + helperClass + ";\n";

  bool inserted = false;
  if (braceOffset < content.size() && content[braceOffset] == '{') {
    content.insert(braceOffset + 1, insertion);
    inserted = true;
  }

  if (!inserted) {
    for (const string &kw : {"class ", "struct "}) {
      size_t pos = content.find(kw + className);
      if (pos == string::npos) continue;
      size_t bracePos = content.find('{', pos);
      if (bracePos == string::npos) continue;
      content.insert(bracePos + 1, insertion);
      inserted = true;
      break;
    }
  }

  if (!inserted) {
    return false;
  }

  string tmpPath = headerPath + ".reflect.tmp";
  ofstream out(tmpPath, ios::trunc | ios::binary);
  if (!out) {
    return false;
  }
  out << content;
  out.close();

  if (rename(tmpPath.c_str(), headerPath.c_str()) != 0) {
    remove(tmpPath.c_str());
    return false;
  }

  return true;
}

class ReflectPragmaHandler : public PragmaHandler {
public:
  ReflectPragmaHandler() : PragmaHandler("reflect") {}

  void HandlePragma(Preprocessor &preprocessor, PragmaIntroducer,
                    Token &firstToken) override {
    SourceLocation loc = firstToken.getLocation();

    if (pendingStart_.isInvalid()) {
      pendingStart_ = loc;
    } else {
      closedRanges_.push_back({pendingStart_, loc});
      pendingStart_ = SourceLocation();
    }

    while (firstToken.isNot(tok::eod)) {
      preprocessor.Lex(firstToken);
    }
  }

  bool covers(SourceLocation loc, SourceManager &sm) const {
    if (loc.isInvalid()) return false;
    SourceLocation fileLoc = sm.getSpellingLoc(loc);
    if (!fileLoc.isFileID()) return false;

    for (const auto &range : closedRanges_) {
      SourceLocation s = sm.getSpellingLoc(range.first);
      SourceLocation e = sm.getSpellingLoc(range.second);
      if (s.isInvalid() || e.isInvalid()) continue;
      if (sm.isBeforeInTranslationUnit(s, fileLoc) &&
          sm.isBeforeInTranslationUnit(fileLoc, e)) {
        return true;
      }
    }

    return false;
  }

private:
  SourceLocation pendingStart_;
  std::vector<std::pair<SourceLocation, SourceLocation>> closedRanges_;
};

struct ClassState {
  CXXRecordDecl *record = nullptr;
  string canonicalName;
  string className;
  string namespaceName;
  string suffix;
  string helperClass;
  string headerPath;
};

struct HeaderState {
  string headerPath;
  string baseName;
  string stem;
  string outFileName;
  vector<unique_ptr<ClassState>> classes;
};

class ReflectVisitor : public RecursiveASTVisitor<ReflectVisitor> {
private:
  ASTContext *astContext_;
  SourceManager &sourceMgr_;
  ReflectPragmaHandler &pragmaHandler_;
  map<string, unique_ptr<HeaderState>> headers_;
  set<CXXRecordDecl *> visitedRecords_;
  set<string> visitedNames_;

public:
  ReflectVisitor(ASTContext *context, ReflectPragmaHandler &pragmaHandler)
      : astContext_(context), sourceMgr_(context->getSourceManager()),
        pragmaHandler_(pragmaHandler) {}

  bool VisitCXXRecordDecl(CXXRecordDecl *Record) {
    if (!Record->isCompleteDefinition() || Record->isImplicit()) {
      return true;
    }
    if (visitedRecords_.count(Record)) {
      return true;
    }
    if (!pragmaHandler_.covers(Record->getBeginLoc(), sourceMgr_)) {
      return true;
    }
    visitedRecords_.insert(Record);

    string canonicalName = Record->getQualifiedNameAsString();
    if (visitedNames_.count(canonicalName)) {
      return true;
    }
    visitedNames_.insert(canonicalName);

    SourceLocation loc = sourceMgr_.getSpellingLoc(Record->getBeginLoc());
    if (!loc.isFileID()) {
      return true;
    }

    const FileEntry *file =
        sourceMgr_.getFileEntryForID(sourceMgr_.getFileID(loc));
    if (!file) {
      return true;
    }

    string headerPath = file->tryGetRealPathName().str();
    if (headerPath.empty()) {
      headerPath = sourceMgr_.getFilename(loc).str();
    }
    if (headerPath.empty() || headerPath.find("reflection") != string::npos) {
      return true;
    }
    headerPath = absolutePath(headerPath);

    auto *state = new ClassState();
    state->record = Record;
    state->canonicalName = canonicalName;
    state->className = Record->getNameAsString();
    state->namespaceName = namespaceOf(Record);
    state->suffix = symbolSuffix(state->canonicalName);
    state->helperClass = "__" + state->className;
    state->headerPath = headerPath;

    SourceLocation lBrace = Record->getBraceRange().getBegin();
    if (lBrace.isValid() && lBrace.isFileID()) {
      unsigned braceOffset = sourceMgr_.getFileOffset(lBrace);
      injectFriend(headerPath, state->className, state->helperClass,
                   braceOffset);
    }

    auto &header = headers_[headerPath];
    if (!header) {
      header = make_unique<HeaderState>();
      header->headerPath = headerPath;
      header->baseName = llvm::sys::path::filename(headerPath).str();
      size_t dot = header->baseName.find_last_of('.');
      header->stem = dot == string::npos
                         ? header->baseName
                         : header->baseName.substr(0, dot);
      header->outFileName =
          "__" + sanitizePath(header->stem) + ".cpp";
    }
    header->classes.emplace_back(state);

    return true;
  }

  void flushGenerated() {
    for (auto &kv : headers_) {
      generateFileForHeader(*kv.second);
    }
  }

  void updateMakefile() {
    const char *kMakefileName = "Makefile.reflect.mk";

    FileLock lock(kMakefileName);
    if (!lock.valid()) {
      return;
    }

    map<string, string> entries;

    {
      ifstream in(kMakefileName);
      if (in) {
        string line;
        const string prefix = "# @reflect: ";
        while (getline(in, line)) {
          if (line.rfind(prefix, 0) == 0) {
            istringstream iss(line.substr(prefix.size()));
            string base, out;
            if (iss >> base >> out) {
              entries[base] = out;
            }
          }
        }
      }
    }

    for (auto &kv : headers_) {
      const HeaderState &h = *kv.second;
      entries[h.baseName] = h.outFileName;
    }

    string tmpName = string(kMakefileName) + ".tmp";
    ofstream out(tmpName, ios::trunc);
    if (!out) {
      llvm::errs() << "[Reflect Plugin] Nao foi possivel criar " << tmpName
                   << "\n";
      return;
    }

    out << "# Gerado automaticamente pelo plugin Clang 'reflect'\n";
    out << "# Nao edite manualmente.\n\n";

    for (const auto &kv : entries) {
      out << "# @reflect: " << kv.first << " " << kv.second << "\n";
    }
    out << "\n";

    out << "REFLECT_GENERATED :=";
    for (const auto &kv : entries) {
      out << " " << kv.second;
    }
    out << "\n\n";

    for (const auto &kv : entries) {
      out << kv.second << ": " << kv.first << "\n";
    }

    out << "\n.PHONY: reflect-clean\n";
    out << "reflect-clean:\n";
    out << "\trm -f";
    for (const auto &kv : entries) {
      out << " " << kv.second;
    }
    out << " " << kMakefileName << "\n";

    out.close();

    if (rename(tmpName.c_str(), kMakefileName) != 0) {
      remove(tmpName.c_str());
    }
  }

private:
  static string classRefExpr(const string &canonicalName) {
    string n = escapeForStringLiteral(canonicalName);
    return "(Class::hasClass(\"" + n + "\")"
           " ? Class::getClass(\"" + n + "\")"
           " : Class::putClass(\"" + n + "\", Class(\"" + n + "\")))";
  }

  void generateFileForHeader(HeaderState &header) {
    string tmpFileName = header.outFileName + ".tmp";

    error_code EC;
    llvm::raw_fd_ostream outFile(tmpFileName, EC, llvm::sys::fs::OF_Text);
    if (EC) {
      llvm::errs() << "[Reflect Plugin] Erro ao criar " << tmpFileName << ": "
                   << EC.message() << "\n";
      return;
    }

    outFile << "// Gerado automaticamente pelo plugin Clang 'reflect'\n";
    outFile << "// Origem: " << header.baseName << "\n\n";
    outFile << "#include \"" << header.baseName << "\"\n";
    outFile << "#include \"Class.hpp\"\n";
    outFile << "#include \"Field.hpp\"\n";
    outFile << "#include \"Method.hpp\"\n";
    outFile << "#include \"Constructor.hpp\"\n";
    outFile << "#include \"Destructor.hpp\"\n";
    outFile << "#include \"Modifier.hpp\"\n";
    outFile << "#include \"Parameter.hpp\"\n";
    outFile << "#include <cstdint>\n";
    outFile << "#include <stdexcept>\n";
    outFile << "#include <type_traits>\n";
    outFile << "#include <typeinfo>\n";
    outFile << "#include <utility>\n";
    outFile << "#include <vector>\n\n";

    for (const auto &cls : header.classes) {
      emitHelperClass(outFile, *cls);
    }

    for (const auto &cls : header.classes) {
      emitRegisterFunction(outFile, *cls);
    }

    outFile.close();

    if (rename(tmpFileName.c_str(), header.outFileName.c_str()) != 0) {
      llvm::errs() << "[Reflect Plugin] Falha ao renomear " << tmpFileName
                   << " -> " << header.outFileName << "\n";
      remove(tmpFileName.c_str());
    }
  }

  void emitHelperClass(llvm::raw_fd_ostream &outFile, const ClassState &cls) {
    CXXRecordDecl *Record = cls.record;
    const string &className = cls.className;
    const string &helperClass = cls.helperClass;
    const string &namespaceName = cls.namespaceName;
    const string qualifiedClass =
        namespaceName.empty() ? className : namespaceName + "::" + className;

    const CXXRecordDecl *polyBase =
        Record->isPolymorphic() ? findPolymorphicBase(Record) : nullptr;
    const bool hasPolyBase = polyBase != nullptr;
    const string polyBaseName =
        hasPolyBase ? polyBase->getQualifiedNameAsString() : string();

    bool hasVirtualDeclared = false;
    for (auto *m : Record->methods()) {
      if (m->isImplicit() || isa<CXXConstructorDecl>(m) ||
          isa<CXXDestructorDecl>(m)) {
        continue;
      }
      if (m->isVirtual()) {
        hasVirtualDeclared = true;
        break;
      }
    }
    const bool generateUpcast = hasPolyBase && hasVirtualDeclared;

    if (!namespaceName.empty()) {
      outFile << "namespace " << namespaceName << " {\n\n";
    }

    outFile << "class " << helperClass << " {\npublic:\n";

    if (generateUpcast) {
      outFile << "    static " << qualifiedClass
              << "* __upcast(void* object) {\n";
      outFile << "        if (object == nullptr) return nullptr;\n";
      outFile << "        auto* base = reinterpret_cast<" << polyBaseName
              << "*>(object);\n";
      outFile << "        return dynamic_cast<" << qualifiedClass
              << "*>(base);\n";
      outFile << "    }\n\n";
    }

    // --- Fields ---
    int fieldIndex = 0;
    for (auto *FieldDecl : Record->fields()) {
      string fieldName = FieldDecl->getNameAsString();
      const bool isPointerField = FieldDecl->getType()->isPointerType();

      outFile << "    static void __field_get_" << fieldIndex
              << "(void* object, void* outValue) {\n";
      outFile << "        auto* instance = reinterpret_cast<" << qualifiedClass
              << "*>(object);\n";
      outFile << "        *static_cast<decltype(" << qualifiedClass << "::"
              << fieldName << ")*>(outValue) = instance->" << fieldName << ";\n";
      outFile << "    }\n\n";

      outFile << "    static void __field_set_" << fieldIndex
              << "(void* object, void* valuePtr) {\n";
      outFile << "        auto* instance = reinterpret_cast<" << qualifiedClass
              << "*>(object);\n";
      if (isPointerField) {
        outFile << "        if (valuePtr == nullptr) {\n";
        outFile << "            instance->" << fieldName << " = nullptr;\n";
        outFile << "        } else {\n";
        outFile << "            instance->" << fieldName
                << " = *static_cast<decltype(" << qualifiedClass << "::"
                << fieldName << ")*>(valuePtr);\n";
        outFile << "        }\n";
      } else {
        outFile << "        if (valuePtr == nullptr) throw "
                   "std::invalid_argument(\"Null value for non-pointer field\");\n";
        outFile << "        instance->" << fieldName
                << " = *static_cast<decltype(" << qualifiedClass << "::"
                << fieldName << ")*>(valuePtr);\n";
      }
      outFile << "    }\n\n";

      fieldIndex++;
    }

    // --- Methods ---
    int methodIndex = 0;
    for (auto *MethodDecl : Record->methods()) {
      if (MethodDecl->isImplicit() || isa<CXXConstructorDecl>(MethodDecl) ||
          isa<CXXDestructorDecl>(MethodDecl)) {
        continue;
      }
      string methodName = MethodDecl->getNameAsString();
      bool isStatic = MethodDecl->isStatic();
      bool isVirtual = MethodDecl->isVirtual();
      bool useUpcast = isVirtual && generateUpcast;

      outFile << "    static void* __method_" << methodIndex << "(";
      if (!isStatic) {
        outFile << "void* object, ";
      }
      outFile << "const std::vector<void*>& args) {\n";

      if (!isStatic) {
        if (useUpcast) {
          outFile << "        auto* instance = __upcast(object);\n";
          outFile << "        if (instance == nullptr) throw std::bad_cast();\n";
        } else {
          outFile << "        auto* instance = reinterpret_cast<"
                  << qualifiedClass << "*>(object);\n";
        }
      }

      outFile << "        if (args.size() != " << MethodDecl->getNumParams()
              << ") throw std::invalid_argument(\"Wrong argument count\");\n";

      for (unsigned i = 0; i < MethodDecl->getNumParams(); ++i) {
        const ParmVarDecl *param = MethodDecl->getParamDecl(i);
        QualType paramType = param->getType();
        const bool isPointerParam = paramType->isPointerType();

        if (isPointerParam) {
          string pointeeType =
              paramType->getPointeeType().getUnqualifiedType().getAsString();
          outFile << "        " << pointeeType << "* arg_" << i
                  << " = nullptr;\n";
          outFile << "        if (args[" << i << "] != nullptr) {\n";
          outFile << "            arg_" << i << " = *static_cast<"
                  << pointeeType << "**>(args[" << i << "]);\n";
          outFile << "        }\n";
        } else {
          string parameterType = paramType.getAsString();
          outFile << "        if (args[" << i
                  << "] == nullptr) throw std::invalid_argument(\"Null "
                     "argument\");\n";
          outFile << "        using __param_" << i << " = " << parameterType
                  << ";\n";
          outFile << "        using __arg_" << i
                  << " = typename std::remove_reference<__param_" << i
                  << ">::type;\n";
          outFile << "        auto& arg_" << i << " = *static_cast<__arg_" << i
                  << "*>(args[" << i << "]);\n";
        }
      }

      outFile << "        ";
      if (isStatic) {
        outFile << qualifiedClass << "::";
      } else {
        outFile << "instance->";
      }
      outFile << methodName << "(";
      for (unsigned i = 0; i < MethodDecl->getNumParams(); ++i) {
        if (i > 0) {
          outFile << ", ";
        }
        outFile << "arg_" << i;
      }
      outFile << ");\n";
      outFile << "        return nullptr;\n";
      outFile << "    }\n\n";
      methodIndex++;
    }

    // --- Destructor ---
    outFile << "    static void __destructor_0(void* object) {\n";
    outFile << "        delete reinterpret_cast<" << qualifiedClass
            << "*>(object);\n";
    outFile << "    }\n\n";

    // --- Constructors ---
    int ctorIndex = 0;
    for (auto *CtorDecl : Record->ctors()) {
      if (CtorDecl->isImplicit() || CtorDecl->isCopyConstructor() ||
          CtorDecl->isMoveConstructor()) {
        continue;
      }
      outFile << "    static void* __constructor_" << ctorIndex
              << "(const std::vector<void*>& args) {\n";
      outFile << "        if (args.size() != " << CtorDecl->getNumParams()
              << ") throw std::invalid_argument(\"Wrong argument count\");\n";
      for (unsigned i = 0; i < CtorDecl->getNumParams(); ++i) {
        const ParmVarDecl *param = CtorDecl->getParamDecl(i);
        QualType paramType = param->getType();
        const bool isPointerParam = paramType->isPointerType();

        if (isPointerParam) {
          string pointeeType =
              paramType->getPointeeType().getUnqualifiedType().getAsString();
          outFile << "        " << pointeeType << "* arg_" << i
                  << " = nullptr;\n";
          outFile << "        if (args[" << i << "] != nullptr) {\n";
          outFile << "            arg_" << i << " = *static_cast<"
                  << pointeeType << "**>(args[" << i << "]);\n";
          outFile << "        }\n";
        } else {
          string parameterType = paramType.getAsString();
          outFile << "        if (args[" << i
                  << "] == nullptr) throw std::invalid_argument(\"Null "
                     "argument\");\n";
          outFile << "        using __ctor_param_" << i << " = " << parameterType
                  << ";\n";
          outFile << "        using __ctor_arg_" << i
                  << " = typename std::remove_reference<__ctor_param_" << i
                  << ">::type;\n";
          outFile << "        auto& arg_" << i << " = *static_cast<__ctor_arg_"
                  << i << "*>(args[" << i << "]);\n";
        }
      }
      outFile << "        " << qualifiedClass << "* obj = new " << qualifiedClass
              << "(";
      for (unsigned i = 0; i < CtorDecl->getNumParams(); ++i) {
        if (i > 0) {
          outFile << ", ";
        }
        outFile << "arg_" << i;
      }
      outFile << ");\n";
      outFile << "        return static_cast<void*>(obj);\n";
      outFile << "    }\n\n";
      ctorIndex++;
    }

    if (ctorIndex == 0) {
      outFile << "    static void* __constructor_0(const std::vector<void*>& "
                 "args) {\n";
      outFile << "        if (!args.empty()) throw std::invalid_argument("
                 "\"Wrong argument count\");\n";
      outFile << "        return static_cast<void*>(new " << qualifiedClass
              << "());\n";
      outFile << "    }\n\n";
      ctorIndex = 1;
    }

    outFile << "};\n\n";

    if (!namespaceName.empty()) {
      outFile << "} // namespace " << namespaceName << "\n\n";
    }
  }

  string typeExpr(QualType qt, bool isVoid) {
    if (isVoid) {
      return "nullptr";
    }
    string name = qualifiedTypeName(qt);
    return "&" + classRefExpr(name);
  }

  void emitRegisterFunction(llvm::raw_fd_ostream &outFile,
                            const ClassState &cls) {
    CXXRecordDecl *Record = cls.record;
    const string &className = cls.className;
    const string &canonicalName = cls.canonicalName;
    const string &helperClass = cls.helperClass;
    const string &namespaceName = cls.namespaceName;
    const string &suffix = cls.suffix;

    string qualifiedHelper =
        namespaceName.empty() ? helperClass : namespaceName + "::" + helperClass;
    string n = escapeForStringLiteral(canonicalName);

    outFile << "extern \"C\" __attribute__((constructor)) void __register_"
            << suffix << "() {\n";
    outFile << "    if (!Class::hasClass(\"" << n << "\")) {\n";
    outFile << "        Class::putClass(\"" << n << "\", Class(\"" << n
            << "\"));\n";
    outFile << "    }\n";
    outFile << "    const Class &self = Class::getClass(\"" << n << "\");\n\n";

    outFile << "    std::vector<const Class*> superclasses = {";
    for (const CXXBaseSpecifier &Base : Record->bases()) {
      const CXXRecordDecl *BaseRecord = Base.getType()->getAsCXXRecordDecl();
      if (BaseRecord != nullptr) {
        outFile << "\n        &" << classRefExpr(
            BaseRecord->getQualifiedNameAsString()) << ",";
      }
    }
    outFile << "\n    };\n\n";

    outFile << "    std::vector<Field> fields = {\n";
    int fieldIndex = 0;
    for (auto *FieldDecl : Record->fields()) {
      outFile << "        Field(&self, Modifier(Modifier::"
              << accessKeyword(FieldDecl->getAccess()) << "), "
              << typeExpr(FieldDecl->getType(), false) << ", \""
              << escapeForStringLiteral(FieldDecl->getNameAsString()) << "\", "
              << "reinterpret_cast<uintptr_t>(&" << qualifiedHelper
              << "::__field_get_" << fieldIndex << "), "
              << "reinterpret_cast<uintptr_t>(&" << qualifiedHelper
              << "::__field_set_" << fieldIndex << ")),\n";
      fieldIndex++;
    }
    outFile << "    };\n\n";

    outFile << "    std::vector<Method> methods = {\n";
    int methodIndex = 0;
    for (auto *MethodDecl : Record->methods()) {
      if (MethodDecl->isImplicit() || isa<CXXConstructorDecl>(MethodDecl) ||
          isa<CXXDestructorDecl>(MethodDecl)) {
        continue;
      }

      outFile << "        Method(&self, Modifier(Modifier::"
              << accessKeyword(MethodDecl->getAccess());
      if (MethodDecl->isStatic()) {
        outFile << " | Modifier::STATIC";
      }
      if (MethodDecl->isConst()) {
        outFile << " | Modifier::CONST";
      }
      if (MethodDecl->isVirtual()) {
        outFile << " | Modifier::VIRTUAL";
      }
      outFile << "), "
              << typeExpr(MethodDecl->getReturnType(),
                          MethodDecl->getReturnType()->isVoidType())
              << ", \""
              << escapeForStringLiteral(MethodDecl->getNameAsString())
              << "\", {";

      for (unsigned i = 0; i < MethodDecl->getNumParams(); ++i) {
        const ParmVarDecl *param = MethodDecl->getParamDecl(i);
        outFile << "\n            Parameter(\""
                << escapeForStringLiteral(param->getNameAsString()) << "\", "
                << typeExpr(param->getType(), false)
                << ", Modifier(Modifier::NONE)),";
      }
      outFile << "\n        }, "
              << "reinterpret_cast<uintptr_t>(&" << qualifiedHelper
              << "::__method_" << methodIndex << ")),\n";
      methodIndex++;
    }
    outFile << "    };\n\n";

    outFile << "    std::vector<Constructor> constructors = {\n";
    int ctorIndex = 0;
    for (auto *CtorDecl : Record->ctors()) {
      if (CtorDecl->isImplicit() || CtorDecl->isCopyConstructor() ||
          CtorDecl->isMoveConstructor()) {
        continue;
      }
      outFile << "        Constructor(&self, Modifier(Modifier::"
              << accessKeyword(CtorDecl->getAccess()) << "), {";
      for (unsigned i = 0; i < CtorDecl->getNumParams(); ++i) {
        const ParmVarDecl *param = CtorDecl->getParamDecl(i);
        outFile << "\n            Parameter(\""
                << escapeForStringLiteral(param->getNameAsString()) << "\", "
                << typeExpr(param->getType(), false)
                << ", Modifier(Modifier::NONE)),";
      }
      outFile << "\n        }, "
              << "reinterpret_cast<uintptr_t>(&" << qualifiedHelper
              << "::__constructor_" << ctorIndex << ")),\n";
      ctorIndex++;
    }
    if (ctorIndex == 0) {
      outFile << "        Constructor(&self, Modifier(Modifier::PUBLIC), {}, "
              << "reinterpret_cast<uintptr_t>(&" << qualifiedHelper
              << "::__constructor_0)),\n";
    }
    outFile << "    };\n\n";

    outFile << "    std::vector<Destructor> destructors = {\n";
    outFile << "        Destructor(reinterpret_cast<uintptr_t>(&"
            << qualifiedHelper << "::__destructor_0)),\n";
    outFile << "    };\n\n";

    outFile << "    Class::putClass(\"" << n << "\",\n";
    outFile << "        Class(\""
            << escapeForStringLiteral(className) << "\", "
            << "Modifier(Modifier::PUBLIC), superclasses, fields, methods, "
            << "constructors, \""
            << escapeForStringLiteral(namespaceName) << "\", destructors));\n";
    outFile << "}\n\n";
  }
};

class ReflectASTConsumer : public ASTConsumer {
private:
  ReflectVisitor visitor_;

public:
  ReflectASTConsumer(ASTContext *context, ReflectPragmaHandler &pragmaHandler)
      : visitor_(context, pragmaHandler) {}

  void HandleTranslationUnit(ASTContext &Context) override {
    visitor_.TraverseDecl(Context.getTranslationUnitDecl());
    visitor_.flushGenerated();
    visitor_.updateMakefile();
  }
};

class ReflectAction : public PluginASTAction {
private:
  ReflectPragmaHandler pragmaHandler_;
  Preprocessor *preprocessor_ = nullptr;

public:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef InFile) override {
    preprocessor_ = &CI.getPreprocessor();
    preprocessor_->AddPragmaHandler(&pragmaHandler_);
    return std::make_unique<ReflectASTConsumer>(&CI.getASTContext(),
                                                pragmaHandler_);
  }

  void EndSourceFileAction() override {
    if (preprocessor_ != nullptr) {
      preprocessor_->RemovePragmaHandler(&pragmaHandler_);
      preprocessor_ = nullptr;
    }
    PluginASTAction::EndSourceFileAction();
  }

  bool ParseArgs(const CompilerInstance &,
                 const std::vector<std::string> &) override {
    return true;
  }
};

} // namespace

static FrontendPluginRegistry::Add<ReflectAction>
    X("reflect", "Zero-boilerplate C++ Reflection Plugin");