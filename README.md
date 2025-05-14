# RttrAutoRegister
RttrAutoRegister is a tool for automatically generating rttr registration code

# License
Licensed under the [Apache License 2.0](LICENSE.txt)

# Usage
## Step 1
Add the defines below to you header file：
```cpp
// Pre-defined macro
#if !defined(RTTR_AUTO_REGISTER_CLASS)
#define RTTR_AUTO_REGISTER_CLASS
#endif

// Pre-defined macro
#if !defined(RTTR_SKIP_REGISTER_PROPERTY)
#define RTTR_SKIP_REGISTER_PROPERTY
#endif

#if !defined(RTTR_REGISTER_FUNCTION_AS_PROPERTY)
#define RTTR_REGISTER_FUNCTION_AS_PROPERTY(propertyName, function)
#endif
```

### RTTR_AUTO_REGISTER_CLASS
Used to mark a class, the tool will automatically register the public members of this class.

Usage is below:
```cpp
class RTTR_AUTO_REGISTER_CLASS TestClass {
 public:
  int a;
  int b;
  int c;
}
```

The generated code is below:
```cpp
registration::class_<TestClass>("TestClass")
    .property("a", &TestClass::a)
	.property("b", &TestClass::b)
	.property("c", &TestClass::c);
```

### RTTR_SKIP_REGISTER_PROPERTY
Used to mark a property of a class, the tool will skip the property while registering.

Usage is below:
```cpp
class RTTR_AUTO_REGISTER_CLASS TestClass {
 public:
  int a;
  int RTTR_SKIP_REGISTER_PROPERTY b;
  int c;
}
```

The generated code is below:
```cpp
registration::class_<TestClass>("TestClass")
    .property("a", &TestClass::a)
	.property("c", &TestClass::c);
```

### RTTR_REGISTER_FUNCTION_AS_PROPERTY
Used to declare the relationship between a property and a function, the tool will register the function as a property.

The usage is below:
```cpp
class RTTR_AUTO_REGISTER_CLASS TestClass {
 public:
  int getA();
  int getB();
  int getC();
  RTTR_REGISTER_FUNCTION_AS_PROPERTY("a", getA)
  RTTR_REGISTER_FUNCTION_AS_PROPERTY("b", getB)
  RTTR_REGISTER_FUNCTION_AS_PROPERTY("c", getC)
 
 private:
  int a;
  int b;
  int c;
}
```

The generated code is below:
```cpp
registration::class_<TestClass>("TestClass")
    .property_readonly("a", &TestClass::getA)
    .property_readonly("b", &TestClass::getB)
    .property_readonly("c", &TestClass::getC);
```

# Example
When using the tool, the following points should be noted:
1. The parameters should ideally be absolute paths
2. If using classes from std, you need to use '-i' to specify the include path of std

Directory can be specified by '-s', the tool will automatically detect all files end with .h and .hpp in that directory:
```
RttrAutoRegister -s /Users/name/rttr-auto-register/test \
                 -o /Users/name/rttr-auto-register/generated/rttrGenerated.h \
                 -m RTTR_TEST_MACRO \
                 -i /Users/name/rttr-auto-register/test /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include
```

Files can also be specified by '-s':
```
RttrAutoRegister -s /Users/name/rttr-auto-register/test/example.h /Users/name/rttr-auto-register/test/example1.h \
                 -o /Users/name/rttr-auto-register/generated/rttrGenerated.h \
                 -m RTTR_TEST_MACRO \
                 -i /Users/name/rttr-auto-register/test /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include
```

