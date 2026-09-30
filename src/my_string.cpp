// ============================================================================
// 作业 2：String 类的实现文件
//
// 目前本文件是空的：构建时链接阶段会报 "undefined reference to `String::...`"，
// 这是预期现象。请先到 include/my_string.h 中补好私有数据成员，再在这里实现
// 所有声明过的成员函数与运算符。
//
// 如果你想拆成多个 .cpp 文件，请同步修改根目录 CMakeLists.txt 中的
// STRING_SOURCES 列表。
//
// 实现清单（与 include/my_string.h 一一对应）：
//   [ ] String() / String(const char*) / 拷贝构造 / 移动构造 / 析构
//   [ ] 复制赋值 operator=(const String&) / 移动赋值 operator=(String&&)
//   [ ] operator+ / operator[]（含 const 版本）/ at（含 const 版本）
//   [ ] size / capacity
//   [ ] insert / push_back
//   [ ] c_str / operator const char*
//   [ ] swap
//   [ ] friend operator<< / operator>>
//
// 完成后按 docs/build-and-test.md 的步骤构建、运行测试并做 ASan/UBSan 检查。
// ============================================================================

#include "my_string.h"
#include <stdexcept>
#include <cctype>
#include <iostream>
namespace {

    std::size_t char_len(const char* s) {
        if (s == nullptr) return 0;
        std::size_t n = 0;
        while (s[n] != '\0') ++n;
        return n;
    }

    void copy_chars(char* dst, const char* src, std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) dst[i] = src[i];
    }

}  
String::String()
    : data_(new char[17]),   
    size_(0),              
    capacity_(16) {        
    data_[0] = '\0';        
}
String::String(const char* str)
    : data_(nullptr),
    size_(char_len(str)),
    capacity_(size_) {        
    data_ = new char[capacity_ + 1];
    if (str != nullptr) {
        copy_chars(data_, str, size_);
        data_[size_] = '\0';
    }
    else {
        data_[0] = '\0';
    }
}
String::~String() {
    delete[] data_;
}
void String::reserve(std::size_t new_cap) {
    if (new_cap <= capacity_) {
        return;                          // 已经够大，什么都不做
    }
    char* new_data = new char[new_cap + 1];   // ① 先分配新的
    copy_chars(new_data, data_, size_);        // ② 拷旧内容
    new_data[size_] = '\0';                    // ③ 补结尾
    delete[] data_;                            // ④ 再释放旧的
    data_ = new_data;                          // ⑤ 换门牌号
    capacity_ = new_cap;
}
void String::push_back(char ch) {
    if (size_ == capacity_) {                     // 满了才扩
        const std::size_t new_cap = (capacity_ == 0) ? 16 : capacity_ * 2;
        reserve(new_cap);                         // 翻倍扩容
    }
    data_[size_] = ch;    // 写新字符
    ++size_;              // 长度 +1
    data_[size_] = '\0';  // 维护"以 '\0' 结尾"
}
std::size_t String::size() const noexcept { return size_; }
std::size_t String::capacity() const noexcept { return capacity_; }

char& String::operator[](std::size_t index) noexcept { return data_[index]; }
const char& String::operator[](std::size_t index) const noexcept { return data_[index]; }

char& String::at(std::size_t index) {
    if (index >= size_) throw std::out_of_range("String::at: index out of range");
    return data_[index];
}
const char& String::at(std::size_t index) const {
    if (index >= size_) throw std::out_of_range("String::at: index out of range");
    return data_[index];
}
const char* String::c_str() const noexcept { return data_; }
String::operator const char*() const noexcept { return data_; }
// ---- M2：拷贝构造（深拷贝）----
String::String(const String& other)
    : data_(new char[other.size_ + 1]),
      size_(other.size_),
      capacity_(other.size_) {
    copy_chars(data_, other.data_, size_ + 1);  // 连结尾 '\0' 一起拷
}
// ---- M2：复制赋值（自赋值安全 + 强异常安全）----
String& String::operator=(const String& other) {
    if (this == &other) {
        return *this;                    // 自赋值 s = s，直接返回
    }
    char* new_data = new char[other.size_ + 1];  // 先分配新的
    copy_chars(new_data, other.data_, other.size_ + 1);
    delete[] data_;                      // 再释放旧的
    data_ = new_data;
    size_ = other.size_;
    capacity_ = other.size_;
    return *this;
}
// ---- M2：拼接（返回新对象，不修改操作数）----
String String::operator+(const String& other) const {
    String result;                        // 默认构造，容量 16
    result.reserve(size_ + other.size_);  // 预留够大
    copy_chars(result.data_, data_, size_);
    copy_chars(result.data_ + size_, other.data_, other.size_);
    result.size_ = size_ + other.size_;
    result.data_[result.size_] = '\0';
    return result;
}
// ---- M2：插入（越界抛异常 + 自插入安全）----
void String::insert(std::size_t pos, const String& str) {
    if (pos > size_) {
        throw std::out_of_range("String::insert: index out of range");
    }
    if (this == &str) {                  // 自插入：先深拷贝一份
        String copy(str);
        insert(pos, copy);
        return;
    }
    const std::size_t new_size = size_ + str.size_;
    if (new_size > capacity_) {
        reserve(new_size);
    }
    for (std::size_t i = size_; i > pos; --i) {   // 从后往前后移
        data_[i - 1 + str.size_] = data_[i - 1];
    }
    for (std::size_t i = 0; i < str.size_; ++i) {
        data_[pos + i] = str.data_[i];
    }
    size_ = new_size;
    data_[size_] = '\0';                 // 重新封口
}
// ---- M3：移动构造（窃取缓冲区）----
String::String(String&& other) noexcept
    : data_(other.data_),          // 直接抢指针
      size_(other.size_),
      capacity_(other.capacity_) {
    other.data_ = new char[1];     // 源对象变回"有效的空串"
    other.data_[0] = '\0';
    other.size_ = 0;
    other.capacity_ = 0;
}

// ---- M3：移动赋值（释放自己的，窃取别人的）----
String& String::operator=(String&& other) noexcept {
    if (this == &other) {          // 自移动 s = std::move(s) 必须安全
        return *this;
    }
    delete[] data_;                // 释放自己的旧缓冲区
    data_ = other.data_;           // 窃取
    size_ = other.size_;
    capacity_ = other.capacity_;
    other.data_ = new char[1];     // 源对象变空壳
    other.data_[0] = '\0';
    other.size_ = 0;
    other.capacity_ = 0;
    return *this;
}
void String::swap(String& other) noexcept {
    // ① 交换缓冲区指针
    char* tmp_d = data_;
    data_ = other.data_;
    other.data_ = tmp_d;

    // ② 交换 size
    std::size_t tmp_s = size_;
    size_ = other.size_;
    other.size_ = tmp_s;

    // ③ 交换 capacity
    std::size_t tmp_c = capacity_;
    capacity_ = other.capacity_;
    other.capacity_ = tmp_c;
}
std::ostream& operator<<(std::ostream& os, const String& str) {
    os << str.data_;   // 友元可以直接碰 data_；它恒有 '\0' 结尾
    return os;
}
std::istream& operator>>(std::istream& is, String& str) {
    // ① 跳过前导空白（空格 / tab / 换行）
    char c;
    while (is.get(c)) {
        if (!std::isspace(static_cast<unsigned char>(c))) {
            break;   // 读到第一个非空白字符
        }
    }

    // ② 关键决策 1：一个字符都没读到（EOF）→ 设 failbit，str 保持原值！
    if (!is) {
        is.setstate(std::ios::failbit);
        return is;
    }

    // ③ 关键决策 2：读到了第一个字符，现在才允许动 str（替换旧内容）
    str = String();      // 清空（复用 M3 的移动赋值，临时对象是右值）
    str.push_back(c);    // 第一个字符（容量不够会自动翻倍，M1 写的）

    // ④ 继续读到空白或 EOF 为止
    while (is.get(c)) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            is.unget();   // 关键决策 3：空白放回流，留给下一次 >>
            break;
        }
        str.push_back(c);
    }
    return is;
}


// TODO: 在此实现 include/my_string.h 中声明的所有成员函数与运算符。
