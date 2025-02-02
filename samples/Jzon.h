/*
Copyright (c) 2013 Johannes Häggqvist

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#ifndef Jzon_h__
#define Jzon_h__

#ifndef JzonAPI
#ifdef _WINDLL
#define JzonAPI __declspec(dllimport)
#elif defined(__GNUC__) && (__GNUC__ >= 4)
#define JzonAPI __attribute__((visibility("default")))
#else
#define JzonAPI
#endif
#endif

#include <iterator>
#include <queue>
#include <stdexcept>
#include <string>
#include <vector>

namespace Jzon {
class Node;
class Value;
class Object;
class Array;
using NamedNode = std::pair<std::string, Node &>;
using NamedNodePtr = std::pair<std::string, Node *>;

class TypeException : public std::logic_error {
 public:
  TypeException() : std::logic_error("A Node was used as the wrong type") {
  }
};
class NotFoundException : public std::out_of_range {
 public:
  NotFoundException() : std::out_of_range("The node could not be found") {
  }
};

struct Format {
  bool newline;
  bool spacing;
  bool useTabs;
  unsigned int indentSize;
};
static const Format StandardFormat = {true, true, true, 1};
static const Format NoFormat = {false, false, false, 0};

class JzonAPI Node {
  friend class Object;
  friend class Array;

 public:
  enum Type { T_OBJECT, T_ARRAY, T_VALUE };

  Node() noexcept = default;
  virtual ~Node() noexcept = default;

  virtual Type GetType() const = 0;

  inline bool IsObject() const {
    return (GetType() == T_OBJECT);
  }
  inline bool IsArray() const {
    return (GetType() == T_ARRAY);
  }
  inline bool IsValue() const {
    return (GetType() == T_VALUE);
  }

  Object &AsObject();
  const Object &AsObject() const;
  Array &AsArray();
  const Array &AsArray() const;
  Value &AsValue();
  const Value &AsValue() const;

  virtual inline bool IsNull() const {
    return false;
  }
  virtual inline bool IsString() const {
    return false;
  }
  virtual inline bool IsNumber() const {
    return false;
  }
  virtual inline bool IsBool() const {
    return false;
  }

  virtual std::string ToString() const {
    throw TypeException();
  }
  virtual int ToInt() const {
    throw TypeException();
  }
  virtual float ToFloat() const {
    throw TypeException();
  }
  virtual double ToDouble() const {
    throw TypeException();
  }
  virtual bool ToBool() const {
    throw TypeException();
  }

  virtual bool Has(const std::string & /*name*/) const {
    throw TypeException();
  }
  virtual size_t GetCount() const {
    return 0;
  }
  virtual Node &Get(const std::string & /*name*/) const {
    throw TypeException();
  }
  virtual Node &Get(size_t /*index*/) const {
    throw TypeException();
  }

  static Type DetermineType(const std::string &json);

 protected:
  virtual Node *GetCopy() const = 0;
};

class JzonAPI Value : public Node {
 public:
  enum ValueType { VT_NULL, VT_STRING, VT_NUMBER, VT_BOOL };

  Value();
  Value(const Value &rhs);
  Value(const Node &rhs);
  Value(ValueType type, const std::string &value);
  Value(const std::string &value);
  Value(const char *value);
  Value(const int value);
  Value(const float value);
  Value(const double value);
  Value(const bool value);
  ~Value() override = default;

  Type GetType() const override;
  ValueType GetValueType() const;

  inline bool IsNull() const override {
    return (type == VT_NULL);
  }
  inline bool IsString() const override {
    return (type == VT_STRING);
  }
  inline bool IsNumber() const override {
    return (type == VT_NUMBER);
  }
  inline bool IsBool() const override {
    return (type == VT_BOOL);
  }

  std::string ToString() const override;
  int ToInt() const override;
  float ToFloat() const override;
  double ToDouble() const override;
  bool ToBool() const override;

  void SetNull();
  void Set(const Value &value);
  void Set(ValueType type, const std::string &value);
  void Set(const std::string &value);
  void Set(const char *value);
  void Set(const int value);
  void Set(const float value);
  void Set(const double value);
  void Set(const bool value);

  Value &operator=(const Value &rhs);
  Value &operator=(const Node &rhs);
  Value &operator=(const std::string &rhs);
  Value &operator=(const char *rhs);
  Value &operator=(const int rhs);
  Value &operator=(const float rhs);
  Value &operator=(const double rhs);
  Value &operator=(const bool rhs);

  bool operator==(const Value &other) const;
  bool operator!=(const Value &other) const;

  static std::string EscapeString(const std::string &value);
  static std::string UnescapeString(const std::string &value);

 protected:
  Node *GetCopy() const override;

 private:
  std::string valueStr;
  ValueType type;
};

static const Value null;

class JzonAPI Object : public Node {
 public:
  class iterator : public std::iterator<std::input_iterator_tag, NamedNode> {
   public:
    iterator(NamedNodePtr *o) : p(o) {
    }
    iterator(const iterator &it) : p(it.p) {
    }

    iterator &operator++() {
      ++p;
      return *this;
    }
    iterator operator++(int) {
      iterator tmp(*this);
      operator++();
      return tmp;
    }

    bool operator==(const iterator &rhs) {
      return p == rhs.p;
    }
    bool operator!=(const iterator &rhs) {
      return p != rhs.p;
    }

    NamedNode operator*() {
      return NamedNode(p->first, *p->second);
    }

   private:
    NamedNodePtr *p;
  };
  class const_iterator : public std::iterator<std::input_iterator_tag, const NamedNode> {
   public:
    const_iterator(const NamedNodePtr *o) : p(o) {
    }
    const_iterator(const const_iterator &it) : p(it.p) {
    }

    const_iterator &operator++() {
      ++p;
      return *this;
    }
    const_iterator operator++(int) {
      const_iterator tmp(*this);
      operator++();
      return tmp;
    }

    bool operator==(const const_iterator &rhs) {
      return p == rhs.p;
    }
    bool operator!=(const const_iterator &rhs) {
      return p != rhs.p;
    }

    const NamedNode operator*() {
      return NamedNode(p->first, *p->second);
    }

   private:
    const NamedNodePtr *p;
  };

  Object() = default;
  Object(const Object &other);
  Object(const Node &other);
  ~Object() override;

  Type GetType() const override;

  void Add(const std::string &name, Node &node);
  void Add(const std::string &name, const Value &node);
  void Remove(const std::string &name);
  void Clear();

  iterator begin();
  const_iterator begin() const;
  iterator end();
  const_iterator end() const;

  bool Has(const std::string &name) const override;
  size_t GetCount() const override;
  Node &Get(const std::string &name) const override;
  using Node::Get;

 protected:
  Node *GetCopy() const override;

 private:
  using ChildList = std::vector<NamedNodePtr>;
  ChildList children;
};

class JzonAPI Array : public Node {
 public:
  class iterator : public std::iterator<std::input_iterator_tag, Node> {
   public:
    iterator(Node **o) : p(o) {
    }
    iterator(const iterator &it) : p(it.p) {
    }

    iterator &operator++() {
      ++p;
      return *this;
    }
    iterator operator++(int) {
      iterator tmp(*this);
      operator++();
      return tmp;
    }

    bool operator==(const iterator &rhs) {
      return p == rhs.p;
    }
    bool operator!=(const iterator &rhs) {
      return p != rhs.p;
    }

    Node &operator*() {
      return **p;
    }

   private:
    Node **p;
  };
  class const_iterator : public std::iterator<std::input_iterator_tag, const Node> {
   public:
    const_iterator(const Node *const *o) : p(o) {
    }
    const_iterator(const const_iterator &it) : p(it.p) {
    }

    const_iterator &operator++() {
      ++p;
      return *this;
    }
    const_iterator operator++(int) {
      const_iterator tmp(*this);
      operator++();
      return tmp;
    }

    bool operator==(const const_iterator &rhs) {
      return p == rhs.p;
    }
    bool operator!=(const const_iterator &rhs) {
      return p != rhs.p;
    }

    const Node &operator*() {
      return **p;
    }

   private:
    const Node *const *p;
  };

  Array() = default;
  Array(const Array &other);
  Array(const Node &other);
  ~Array() override;

  Type GetType() const override;

  void Add(Node &node);
  void Add(const Value &node);
  void Remove(size_t index);
  void Clear();

  iterator begin();
  const_iterator begin() const;
  iterator end();
  const_iterator end() const;

  size_t GetCount() const override;
  Node &Get(size_t index) const override;
  using Node::Get;

 protected:
  Node *GetCopy() const override;

 private:
  using ChildList = std::vector<Node *>;
  ChildList children;
};

class JzonAPI FileWriter {
 public:
  FileWriter(std::string filename);
  ~FileWriter() = default;

  static void WriteFile(const std::string &filename, const Node &root, const Format &format = NoFormat);

  void Write(const Node &root, const Format &format = NoFormat);

 private:
  std::string filename;
};

class JzonAPI FileReader {
 public:
  FileReader(const std::string &filename);
  ~FileReader() = default;

  static bool ReadFile(const std::string &filename, Node &node);

  bool Read(Node &node);

  Node::Type DetermineType();

  const std::string &GetError() const;

 private:
  static bool loadFile(const std::string &filename, std::string &json);
  std::string json;
  std::string error;
};

class JzonAPI Writer {
 public:
  Writer(const Node &root, const Format &format = NoFormat);
  ~Writer();

  void SetFormat(const Format &format);
  const std::string &Write();

  // Return result from last call to Write()
  const std::string &GetResult() const;

  // Disable assignment operator
  Writer &operator=(const Writer &) = delete;

 private:
  void writeNode(const Node &node, size_t level);
  void writeObject(const Object &node, size_t level);
  void writeArray(const Array &node, size_t level);
  void writeValue(const Value &node);

  std::string result;

  class FormatInterpreter *fi;

  const Node &root;
};

class JzonAPI Parser {
 public:
  Parser(Node &root);
  Parser(Node &root, const std::string &json);
  ~Parser() = default;

  void SetJson(const std::string &json);
  bool Parse();

  const std::string &GetError() const;

  // Disable assignment operator
  Parser &operator=(const Parser &) = delete;

 private:
  enum Token {
    T_UNKNOWN,
    T_OBJ_BEGIN,
    T_OBJ_END,
    T_ARRAY_BEGIN,
    T_ARRAY_END,
    T_SEPARATOR_NODE,
    T_SEPARATOR_NAME,
    T_VALUE
  };

  void tokenize();
  bool assemble();

  char peek();
  void jumpToNext(char c);
  void jumpToCommentEnd();

  void readString();
  bool interpretValue(const std::string &value);

  std::string json;
  std::size_t jsonSize{0};

  std::queue<Token> tokens;
  std::queue<std::pair<Value::ValueType, std::string>> data;

  std::size_t cursor{0};

  Node &root;

  std::string error;
};
}  // namespace Jzon

#endif  // Jzon_h__
