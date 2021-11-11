#pragma once

#include "base.h"

namespace Memory
{

class Operand {
public:
  Operand(const string& op) : isUsed_(true)
  {
    if (op.empty())
      isUsed_ = false;
    else {
      smatch m;
      regex e("(\\[?) *([\\w]{2,} *\\+?)? *([\\w]{2,} *\\* *[\\d]+ *\\+?)? *([\\w]+)? *(\\]?)");
      regex_match(op, m, e);
      reg_ = m[2];
      sib_ = m[3];
      disp_ = m[4];

      FindDisp_();
      if (m[1] == "[") {
        if (m[5] == "]") {
          type_ = 'm';
          size_ = 'm';
        }
        else
          _throws("Bad operand format");
      }
      else if (reg_.empty()) {
        type_ = 'i';
        size_ = dispSize_;
      }
      else {
        type_ = 'r';
        FindRegSize_();
      }
    }
  }

  constexpr string GetRegister() const noexcept
  {
    return reg_;
}

  constexpr string GetScalarIndex() const noexcept
  {
    return sib_;
  }

  constexpr string GetDisplacement() const noexcept
  {
    return disp_;
  }

  constexpr ubyte_t GetDispSize() const noexcept
  {
    return dispSize_;
  }

  constexpr ubyte_t GetDispByte() const noexcept
  {
    return disp8_;
  }

  constexpr ushort_t GetDispShort() const noexcept
  {
    return disp16_;
  }

  constexpr ulong_t GetDispLong() const noexcept
  {
    return disp32_;
  }

  constexpr uquad_t GetDispQuad() const noexcept
  {
    return disp64_;
  }

  constexpr ubyte_t GetType() const noexcept
  {
    return type_;
  }

  constexpr ubyte_t GetSize() const noexcept
  {
    return size_;
  }

  constexpr bool IsUsed() const noexcept
  {
    return isUsed_;
  }

  friend constexpr bool operator==(const Operand& l, const Operand& r)
  {
    return (l.reg_ == r.reg_ && l.sib_ == r.sib_ && l.disp_ == r.disp_);
  }

private:
  string reg_;
  string sib_;
  string disp_;
  union {
    uint8_t disp8_;
    uint16_t disp16_;
    uint32_t disp32_;
    uint64_t disp64_;
  };
  ubyte_t dispSize_;
  ubyte_t type_;
  ubyte_t size_;
  bool isUsed_;

  void FindDisp_()
  {
    if (sib_.empty() && disp_.empty() && !reg_.empty()) {
      if (isdigit(reg_[0]) || reg_[0] == '-') {
        disp_ = reg_;
        reg_.clear();
      }
    }
    if (!disp_.empty()) {
      disp64_ = stoull(disp_, nullptr, 0);
      if (disp64_ <= numeric_limits<uint8_t>::max())
        dispSize_ = 'b';
      else if (disp64_ <= numeric_limits<uint16_t>::max())
        dispSize_ = 'w';
      else if (disp64_ <= numeric_limits<uint32_t>::max())
        dispSize_ = 'l';
      else
        dispSize_ = 'q';
    }
  }

  void FindRegSize_()
  {
    if (!reg_.empty()) {
      if (reg_[0] == 'r')
        size_ = 'q';
      else if (reg_[0] == 'e')
        size_ = 'l';
      else if (reg_[1] == 'x' || reg_[1] == 'p' || reg_[1] == 'i')
        size_ = 'w';
      else if (reg_[1] == 'l' || reg_[1] == 'h')
        size_ = 'b';
      else
        size_ = 'l';
    }
  }
};

class Opcode {
public:
  Opcode(const string& bytes, const ubyte_t& rm, const string& mnemonic,
         const string& left, const string& right) :
    bytes_(bytes), rm_(rm), mnemonic_(mnemonic), left_(left), right_(right)
  {
  }

  const string GetBytes(const Operand left, const Operand right) const
  {
    auto r = (right.GetType() == 'r') ? right : left;
    auto m = (r == right) ? left : right;

    ubyte_t rm;
    if (rm_ == 'r')
      rm = FindRmDigit_(r.GetRegister());
    else
      rm = rm_;

    string result = bytes_;
    ubyte_t t = FindModDigit_(m);
    ubyte_t modrm = ((t & 24) << 3) | (rm << 3) | (t & 7);
    result += ' ';
    result += to_string(modrm);
    result += ' ';

    if (!m.GetScalarIndex().empty()) {
      t = FindSibDigit_(m);
      result += to_string(t);
    }

    t = m.GetDispSize();
    if (t == 'b')
      result += to_string(m.GetDispByte());
    else if (t == 'w')
      result += to_string(m.GetDispShort());
    else if (t == 'l')
      result += to_string(m.GetDispLong());
    else if (t == 'q')
      result += to_string(m.GetDispQuad());

    return result;
  }

  constexpr bool IsMatch(const string& mnemonic, const Operand& left,
                         const Operand& right) const noexcept
  {
    if (mnemonic_ == mnemonic) {
      if (!left_.empty() && left.IsUsed()) {
        if (left_.find_first_of(left.GetType()) == string::npos ||
            left_.find_first_of(left.GetSize()) == string::npos)
          return false;
        else if (!right_.empty() && right.IsUsed()) {
          if (right_.find_first_of(right.GetType()) == string::npos ||
              right_.find_first_of(right.GetSize()) == string::npos)
            return false;
          else
            return true;
        }
      }
    }
    return false;
  }

private:
  inline static const vector<vector<string>> registers_ = {
    { "al", "ax", "eax", "st0", "mm0", "xmm0", "es", "cr0", "dr0" },
    { "cl", "cx", "ecx", "st1", "mm1", "xmm1", "cs", "dr1" },
    { "dl", "dx", "edx", "st2", "mm2", "xmm2", "ss", "cr2", "dr2" },
    { "bl", "bx", "ebx", "st3", "mm3", "xmm3", "ds", "cr3", "dr3" },
    { "ah", "sp", "esp", "st4", "mm4", "xmm4", "fs", "cr4", "dr4" },
    { "ch", "bp", "ebp", "st5", "mm5", "xmm5", "gs", "dr5" },
    { "dh", "si", "esi", "st6", "mm6", "xmm6", "dr6" },
    { "bh", "di", "edi", "st7", "mm7", "xmm7", "dr7" }
  };

  string bytes_;
  ubyte_t rm_;
  string mnemonic_;
  string left_;
  string right_;

  ubyte_t FindRmDigit_(const string& reg) const
  {
    ubyte_t b = 0;
    bool found = false;
    for (; b < static_cast<ubyte_t>(registers_.size()); ++b) {
      for (auto r = registers_[b].begin(); r != registers_[b].end(); ++r) {
        if (reg.find(*r) != string::npos) {
          found = true;
          break;
        }
      }
      if (found)
        break;
    }
    if (!found)
      _throws("Register mnemonic not found");
    return b;
  }

  ubyte_t FindModDigit_(const Operand& op) const
  {
    if (op.GetType() == 'r')
      return (3 << 3) | FindRmDigit_(op.GetRegister());

    ubyte_t b = ((op.GetDispSize() == 'b') ? 1 : 2) << 3;
    if (!op.GetScalarIndex().empty()) {
      if (!op.GetDisplacement().empty() && !op.GetRegister().empty())
        return b | 4;
      else
        return 4;
    }
    else if (!op.GetRegister().empty()) {
      if (!op.GetDisplacement().empty())
        return b | FindRmDigit_(op.GetRegister());
      else
        return FindRmDigit_(op.GetRegister());
    }
    else if (!op.GetDisplacement().empty())
      return 5;
    else
      _throws("Unable to find operand mod digit");
    return 0;
  }

  ubyte_t FindSibDigit_(const Operand& op) const
  {
    smatch sm;
    regex e("([\\w]+) *\\*? *(\\d?) *\\+");
    string s = op.GetScalarIndex();
    regex_match(s, sm, e);

    ubyte_t b = 0;
    if (!sm[2].str().empty()) {
      ubyte_t scalar = static_cast<ubyte_t>(stoi(sm[2].str()));
      if (scalar == 2)
        b = 1 << 3;
      else if (scalar == 4)
        b = 2 << 3;
      else if (scalar == 8)
        b = 3 << 3;
      else
        _throws("Invalid scalar value used (only use 2, 4, or 8");
    }

    if (!sm[1].str().empty())
      b |= FindRmDigit_(sm[1].str());
    else
      _throws("Unable to find scalar index byte");

    if (!op.GetRegister().empty())
      b = (b << 3) | FindRmDigit_(op.GetRegister());
    else
      b = (b << 3) | 5;
    return b;
  }
};

class Instruction {
public:
  Instruction(const string& opcode, Operand& left, Operand& right) :
    opcode_(opcode), left_(left), right_(right)
  {
    auto op = opcodes_.begin();
    for (; op != opcodes_.end(); ++op) {
      if (op->IsMatch(opcode, left, right))
        break;
    }
    if (op == opcodes_.end())
      _throws("Unable to find opcode");
    cout << op->GetBytes(left, right) << endl;
  }

private:
  inline static const vector<Opcode> opcodes_ = {
    {
      { "0", 'r', "add", "r/m b", "r b"},
      { "1", 'r', "add", "r/m l", "r l"},
      { "66 1", 'r', "add", "r/m w", "r w"},
      { "2", 'r', "add", "r b", "r/m b"},
      { "3", 'r', "add", "r l", "r/m l"},
      { "66 3", 'r', "add", "r/m w", "r w"}
    }
  };

  string opcode_;
  Operand left_;
  Operand right_;
};

class Patch {
public:
  Patch(int address) : address_(address), symbols_({})
  {
  }

  void Symbols(initializer_list<pair<const string, int>> il)
  {
    symbols_.insert(il);
  }

  void Assembly(const string& code)
  {
    // TODO: parse symbols before instructions
    smatch ms;
    regex e("([\\w]+) *([^\\,\\n]*) *\\,? *([^\\n]*) *(?=\\n)");

    auto s = code;
    while (regex_search(s, ms, e)) {
      Operand l(ms[2].str());
      Operand r(ms[3].str());
      Instruction(ms[1].str(), l, r);
      s = ms.suffix().str();
    }
  }

private:
  int address_;
  vector<Instruction> inst_;
  map<const string, int> symbols_;
};

}
