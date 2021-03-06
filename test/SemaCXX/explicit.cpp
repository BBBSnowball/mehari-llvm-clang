// RUN: %clang_cc1 -fsyntax-only -verify -std=c++11 %s
namespace Constructor {
struct A {
  A(int);
};

struct B { // expected-note+ {{candidate}}
  explicit B(int);
};

B::B(int) { } // expected-note+ {{here}}

struct C {
  void f(const A&);
  void f(const B&);
};

void f(C c) {
  c.f(10);
}

A a0 = 0;
A a1(0);
A &&a2 = 0;
A &&a3(0);
A a4{0};
A &&a5 = {0};
A &&a6{0};

B b0 = 0; // expected-error {{no viable conversion}}
B b1(0);
B &&b2 = 0; // expected-error {{could not bind}}
B &&b3(0); // expected-error {{could not bind}}
B b4{0};
B &&b5 = {0}; // expected-error {{chosen constructor is explicit}}
B &&b6{0};
}

namespace Conversion {
  struct A {
    operator int();
    explicit operator bool();
  };

  A::operator bool() { return false; } 

  struct B {
    void f(int);
    void f(bool);
  };

  void f(A a, B b) {
    b.f(a);
  }
  
  void testExplicit()
  {
    // Taken from 12.3.2p2
    class X { X(); }; // expected-note+ {{candidate constructor}}
    class Y { }; // expected-note+ {{candidate constructor (the implicit}}

    struct Z {
      explicit operator X() const;
      explicit operator Y() const;
      explicit operator int() const;
    };
    
    Z z;
    // 13.3.1.4p1 & 8.5p16:
    Y y2 = z; // expected-error {{no viable conversion from 'Z' to 'Y'}}
    Y y2b(z);
    Y y3 = (Y)z;
    Y y4 = Y(z);
    Y y5 = static_cast<Y>(z);
    // 13.3.1.5p1 & 8.5p16:
    int i1 = (int)z;
    int i2 = int(z);
    int i3 = static_cast<int>(z);
    int i4(z);
    // 13.3.1.6p1 & 8.5.3p5:
    const Y& y6 = z; // expected-error {{no viable conversion from 'Z' to 'const Y'}}
    const int& y7 = z; // expected-error {{no viable conversion from 'Z' to 'const int'}}
    const Y& y8(z);
    const int& y9(z);

    // Y is an aggregate, so aggregate-initialization is performed and the
    // conversion function is not considered.
    const Y y10{z}; // expected-error {{excess elements}}
    const Y& y11{z}; // expected-error {{no viable conversion from 'Z' to 'const Y'}}
    const int& y12{z};

    // X is not an aggregate, so constructors are considered.
    // However, by 13.3.3.1/4, only standard conversion sequences and
    // ellipsis conversion sequences are considered here, so this is not
    // allowed.
    // FIXME: It's not really clear that this is a sensible restriction for this
    // case. g++ allows this, EDG does not.
    const X x1{z}; // expected-error {{no matching constructor}}
    const X& x2{z}; // expected-error {{no matching constructor}}
  }
  
  void testBool() {
    struct Bool {
      operator bool();
    };

    struct NotBool {
      explicit operator bool(); // expected-note {{conversion to integral type 'bool'}}
    };
    Bool    b;
    NotBool n;

    (void) (1 + b);
    (void) (1 + n); // expected-error {{invalid operands to binary expression ('int' and 'NotBool')}}
    
    // 5.3.1p9:
    (void) (!b);
    (void) (!n);
    
    // 5.14p1:
    (void) (b && true);
    (void) (n && true);
    
    // 5.15p1:
    (void) (b || true);
    (void) (n || true);
    
    // 5.16p1:
    (void) (b ? 0 : 1);
    (void) (n ? 0: 1);
    
    // 5.19p5:
    // TODO: After constexpr has been implemented
    
    // 6.4p4:
    if (b) {}
    if (n) {}
    
    // 6.4.2p2:
    switch (b) {} // expected-warning {{switch condition has boolean value}}
    switch (n) {} // expected-error {{switch condition type 'NotBool' requires explicit conversion to 'bool'}} \
                     expected-warning {{switch condition has boolean value}}
    
    // 6.5.1:
    while (b) {}
    while (n) {}
    
    // 6.5.2p1:
    do {} while (b);
    do {} while (n);
    
    // 6.5.3:
    for (;b;) {}
    for (;n;) {}

    // 13.3.1.5p1:
    bool direct1(b);
    bool direct2(n);
    int direct3(b);
    int direct4(n); // expected-error {{no viable conversion}}
    const bool &direct5(b);
    const bool &direct6(n);
    const int &direct7(b);
    const int &direct8(n); // expected-error {{no viable conversion}}
    bool directList1{b};
    bool directList2{n};
    int directList3{b};
    int directList4{n}; // expected-error {{no viable conversion}}
    const bool &directList5{b};
    const bool &directList6{n};
    const int &directList7{b};
    const int &directList8{n}; // expected-error {{no viable conversion}}
    bool copy1 = b;
    bool copy2 = n; // expected-error {{no viable conversion}}
    int copy3 = b;
    int copy4 = n; // expected-error {{no viable conversion}}
    const bool &copy5 = b;
    const bool &copy6 = n; // expected-error {{no viable conversion}}
    const int &copy7 = b;
    const int &copy8 = n; // expected-error {{no viable conversion}}
    bool copyList1 = {b};
    bool copyList2 = {n}; // expected-error {{no viable conversion}}
    int copyList3 = {b};
    int copyList4 = {n}; // expected-error {{no viable conversion}}
    const bool &copyList5 = {b};
    const bool &copyList6 = {n}; // expected-error {{no viable conversion}}
    const int &copyList7 = {b};
    const int &copyList8 = {n}; // expected-error {{no viable conversion}}
  }
  
  void testNew()
  {
    // 5.3.4p6:
    struct Int {
      operator int();
    };
    struct NotInt {
      explicit operator int(); // expected-note {{conversion to integral type 'int' declared here}}
    };
    
    Int    i;
    NotInt ni;
    
    new int[i];
    new int[ni]; // expected-error {{array size expression of type 'NotInt' requires explicit conversion to type 'int'}}
  }
  
  void testDelete()
  {
    // 5.3.5pp2:
    struct Ptr {
      operator int*();
    };
    struct NotPtr {
      explicit operator int*(); // expected-note {{conversion}}
    };
    
    Ptr    p;
    NotPtr np;
    
    delete p;
    delete np; // expected-error {{converting delete expression from type 'NotPtr' to type 'int *' invokes an explicit conversion function}}
  }
  
  void testFunctionPointer()
  {
    // 13.3.1.1.2p2:
    using Func = void(*)(int);
    
    struct FP {
      operator Func();
    };
    struct NotFP {
      explicit operator Func();
    };
    
    FP    fp;
    NotFP nfp;
    fp(1);
    nfp(1); // expected-error {{type 'NotFP' does not provide a call operator}}
  }
}
