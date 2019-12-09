#ifndef MEMORY_REPLAY_VT_HXX
#define MEMORY_REPLAY_VT_HXX

#include <sstream>
#include <string>

class VT {
public:
    explicit VT(std::string vtText);
    uint32_t    getField0() const {return this->m_field0;};
    uint32_t    getField1() const {return this->m_field1;};
    double      getField2() const {return this->m_field2;};
    double      getField3() const {return this->m_field3;};
    double      getField4() const {return this->m_field4;};
    uint32_t    getField5() const {return this->m_field5;};
private:
    uint32_t    m_field0;
    uint32_t    m_field1;
    double      m_field2;
    double      m_field3;
    double      m_field4;
    uint32_t    m_field5;
};

inline std::ostream & operator<<(std::ostream & Str, const VT & vt) { 
  Str << "{" << vt.getField0() << ", " << vt.getField1() << ", " << vt.getField2();
  Str << ", " << vt.getField3() << ", " << vt.getField4() << ", " << vt.getField5() << "}";
  return Str;
}

#endif // MEMORY_REPLAY_VT_HXX