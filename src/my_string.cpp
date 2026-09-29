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




// TODO: 在此实现 include/my_string.h 中声明的所有成员函数与运算符。
