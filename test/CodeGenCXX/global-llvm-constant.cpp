// RUN: %clang_cc1 -emit-llvm -o - %s | FileCheck %s

struct A {
  A() { x = 10; }
  int x;
};

const A x;

// CHECK: @x = internal global
