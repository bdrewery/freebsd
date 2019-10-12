//===--- Demangle.h ---------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_DEMANGLE_DEMANGLE_H
#define LLVM_DEMANGLE_DEMANGLE_H

#include <cstddef>
#include <string>

namespace llvm {
/// This is a llvm local version of __cxa_demangle. Other than the name and
/// being in the llvm namespace it is identical.
///
/// The mangled_name is demangled into buf and returned. If the buffer is not
/// large enough, realloc is used to expand it.
///
/// The *status will be set to a value from the following enumeration
enum : int {
  demangle_unknown_error = -4,
  demangle_invalid_args = -3,
  demangle_invalid_mangled_name = -2,
  demangle_memory_alloc_failure = -1,
  demangle_success = 0,
};

char *itaniumDemangle(const char *mangled_name, char *buf, size_t *n,
                      int *status);


enum MSDemangleFlags { MSDF_None = 0, MSDF_DumpBackrefs = 1 << 0 };
char *microsoftDemangle(const char *mangled_name, char *buf, size_t *n,
                        int *status, MSDemangleFlags Flags = MSDF_None);

/// Attempt to demangle a string using different demangling schemes.
/// The function uses heuristics to determine which demangling scheme to use.
/// \param MangledName - reference to string to demangle.
/// \returns - the demangled string, or a copy of the input string if no
/// demangling occurred.
std::string demangle(const std::string &MangledName);

/// "Partial" demangler. This supports demangling a string into an AST
/// (typically an intermediate stage in itaniumDemangle) and querying certain
/// properties or partially printing the demangled name.
struct ItaniumPartialDemangler {
  ItaniumPartialDemangler();

  ItaniumPartialDemangler(ItaniumPartialDemangler &&Other);
  ItaniumPartialDemangler &operator=(ItaniumPartialDemangler &&Other);

  /// Demangle into an AST. Subsequent calls to the rest of the member functions
  /// implicitly operate on the AST this produces.
  /// \return true on error, false otherwise
  bool partialDemangle(const char *MangledName);

  /// Just print the entire mangled name into Buf. Buf and N behave like the
  /// second and third parameters to itaniumDemangle.
  char *finishDemangle(char *Buf, size_t *N) const;

  /// Get the base name of a function. This doesn't include trailing template
  /// arguments, ie for "a::b<int>" this function returns "b".
  char *getFunctionBaseName(char *Buf, size_t *N) const;

  /// Get the context name for a function. For "a::b::c", this function returns
  /// "a::b".
  char *getFunctionDeclContextName(char *Buf, size_t *N) const;

  /// Get the entire name of this function.
  char *getFunctionName(char *Buf, size_t *N) const;

  /// Get the parameters for this function.
  char *getFunctionParameters(char *Buf, size_t *N) const;
  char *getFunctionReturnType(char *Buf, size_t *N) const;

  /// If this function has any any cv or reference qualifiers. These imply that
  /// the function is a non-static member function.
  bool hasFunctionQualifiers() const;

  /// If this symbol describes a constructor or destructor.
  bool isCtorOrDtor() const;

  /// If this symbol describes a function.
  bool isFunction() const;

  /// If this symbol describes a variable.
  bool isData() const;

  /// If this symbol is a <special-name>. These are generally implicitly
  /// generated by the implementation, such as vtables and typeinfo names.
  bool isSpecialName() const;

  ~ItaniumPartialDemangler();
private:
  void *RootNode;
  void *Context;
};
} // namespace llvm

#endif
