//
//  DoubleArrayUnit.hpp
//
//  Created by 松本拓真 on 2019/08/06.
//

#ifndef DoubleArrayUnit_hpp
#define DoubleArrayUnit_hpp

#include "bit_util256.hpp"

namespace sim_ds {


template <class T>
class _DoubleArrayUnitTraits {
 public:
  using index_type = typename T::_index_type;
  using inset_type = typename T::_inset_type;
  static constexpr size_t kUnitSize = T::kUnitSize;
  static constexpr index_type kEmptyFlag = T::kEmptyFlag;
  [[maybe_unused]] inset_type pred() const {return static_cast<T &>(this)->pred();}
  [[maybe_unused]] inset_type succ() const {return static_cast<T &>(this)->succ();}
};


// MARK: - Unit reference

//  Double-array unit (Base/Check/LabelId) Implementation
//      Base: Pointer
//          if disabled:
//              1 0 0 *(pred)
//          else if basic base:
//              0 0 0 *(base)
//          else if label id:
//              if suffix:
//              0 1 1 *(label id)
//              else:
//              0 1 0 *(label id)
//      Check: Pointer
//          if disabled:
//              1 0 *(succ)
//          else:
//              0 ?(terminal flag) *(check)
//  Paremers for siblings link
//      Child: Byte
//          *(child char)
//      Sibling: Byte
//          *(child char)
//
template<class _Ctnr>
class _DoubleArrayUnitBasicReferenceCommon {
 public:
  using _index_type = typename _Ctnr::index_type;
  using _char_type = typename _Ctnr::char_type;
  using _inset_type = typename _Ctnr::inset_type;

  static constexpr size_t kIndexSize = sizeof(_index_type);
  static constexpr _index_type kUpperBit = 1ull << (kIndexSize * 8 - 1);
  static constexpr _index_type kSecondBit = kUpperBit >> 1;
  static constexpr _index_type kThirdBit = kUpperBit >> 2;
  static constexpr _index_type kEmptyFlag = kUpperBit;
  static constexpr _index_type kTerminalFlag = kSecondBit;
  static constexpr _index_type kLabelFlag = kSecondBit;
  static constexpr _index_type kSuffixFlag = kThirdBit;
  static constexpr _index_type kIndexMask = kThirdBit - 1;
  static constexpr _char_type kEmptyChar = 0xff;

  static constexpr size_t kCheckInsets = 0;
  static constexpr size_t kEmptyFlagInsets = kCheckInsets + kIndexSize - 1;
  static constexpr size_t kSiblingInsets = kIndexSize;
  static constexpr size_t kBaseInsets = kIndexSize + 1;
  static constexpr size_t kChildInsets = kIndexSize * 2 + 1;
  static constexpr size_t kEdgeFlagInsets = kBaseInsets + kIndexSize - 1;
  static constexpr size_t kLabelFlagInsets = kEdgeFlagInsets;
  static constexpr size_t kSuffixFlagInsets = kLabelFlagInsets;
  static constexpr size_t kSuccInsets = kBaseInsets;
  static constexpr size_t kPredInsets = kCheckInsets;
  static constexpr size_t kUnitSize = kIndexSize * 2 + 2;
};


template<class _Ctnr>
class _DoubleArrayUnitBasicConstReference;

template<class _Ctnr>
class _DoubleArrayUnitBasicReference : public _DoubleArrayUnitBasicReferenceCommon<_Ctnr> {
  using _common = _DoubleArrayUnitBasicReferenceCommon<_Ctnr>;

  using _unit_storage_type = typename _Ctnr::_unit_storage_type;
  static_assert(sizeof(_unit_storage_type) == 1, "Invalid unit storage type!");
  using _unit_storage_pointer = typename _Ctnr::_unit_storage_pointer;

  using typename _common::_index_type;
  using typename _common::_char_type;

 private:
  _unit_storage_pointer pointer_;

  friend typename _Ctnr::_self;

  friend class _DoubleArrayUnitBasicConstReference<_Ctnr>;

  template<size_t Offset>
  _index_type _index() const { return *reinterpret_cast<const _index_type *>(pointer_ + Offset); }

  template<size_t Offset>
  _index_type &_index() { return *reinterpret_cast<_index_type *>(pointer_ + Offset); }

 public:
  _index_type check() const { return _index<_common::kCheckInsets>() bitand _common::kIndexMask; }

  void set_check(_index_type new_check) {
    _index_type &target = _index<_common::kCheckInsets>();
    target = new_check bitor (target bitand _common::kTerminalFlag);
  }

  _index_type target() const { return _index<_common::kBaseInsets>() bitand _common::kIndexMask; }

  _index_type base() const { return target(); }

  void set_base(_index_type new_base) {
    _index<_common::kBaseInsets>() = new_base bitand _common::kIndexMask;
  }

  bool has_label() const { return *(pointer_ + _common::kLabelFlagInsets) bitand 0x40; }

  bool label_is_suffix() const { return *(pointer_ + _common::kSuffixFlagInsets) bitand 0x20; }

  _index_type pool_index() const {
    assert(has_label());
    return target();
  }

  void set_pool_index(_index_type new_pool_index, bool label_is_suffix) {
    _index<_common::kBaseInsets>() = ((new_pool_index bitand _common::kIndexMask) bitor
        _common::kLabelFlag bitor
        (label_is_suffix ? _common::kSuffixFlag : 0));
  }

  _char_type child() const { return *(pointer_ + _common::kChildInsets); }

  void set_child(_char_type new_child) {
    *(pointer_ + _common::kChildInsets) = new_child;
  }

  _char_type sibling() const { return *(pointer_ + _common::kSiblingInsets); }

  void set_sibling(_char_type new_sibling) {
    *(pointer_ + _common::kSiblingInsets) = new_sibling;
  }

  bool base_empty() const { return *(pointer_ + _common::kEdgeFlagInsets) bitand 0x80; }

  bool check_empty() const { return *(pointer_ + _common::kEmptyFlagInsets) bitand 0x80; }

  bool is_leaf() const {return base_empty() or (has_label() and label_is_suffix());}

  _index_type pred() const {
    assert(_index<_common::kPredInsets>() bitand _common::kEmptyFlag);
    return _index<_common::kPredInsets>() bitand _common::kIndexMask;
  }

  void set_pred(_index_type new_pred) {
    _index<_common::kPredInsets>() = (new_pred bitand _common::kIndexMask) bitor _common::kEmptyFlag;
  }

  _index_type succ() const {
    assert(_index<_common::kSuccInsets>() bitand _common::kEmptyFlag);
    return _index<_common::kSuccInsets>() bitand _common::kIndexMask;
  }

  void set_succ(_index_type new_succ) {
    _index<_common::kSuccInsets>() = (new_succ bitand _common::kIndexMask) bitor _common::kEmptyFlag;
  }

  void clean_with_link(uint8_t pred, uint8_t succ) {
    init_disabled_unit(pred, succ);
  }

  void init_disabled_unit(uint8_t pred, uint8_t succ) {
    set_pred(pred);
    set_sibling(_common::kEmptyChar);
    set_succ(succ);
    set_child(_common::kEmptyChar);
  }

  void init_unit() {
    _index<_common::kCheckInsets>() = _common::kEmptyFlag;
    set_sibling(_common::kEmptyChar);
    _index<_common::kBaseInsets>() = _common::kEmptyFlag;
    set_child(_common::kEmptyChar);
  }

  void init_unit(_index_type check, _char_type sibling) {
    set_check(check);
    set_sibling(sibling);
    _index<_common::kBaseInsets>() = _common::kEmptyFlag;
    set_child(_common::kEmptyChar);
  }

 private:
  _DoubleArrayUnitBasicReference(_unit_storage_pointer pointer) : pointer_(pointer) {}

};


template<class _Da>
class _DoubleArrayUnitBasicConstReference : public _DoubleArrayUnitBasicReferenceCommon<_Da> {
  using _common = _DoubleArrayUnitBasicReferenceCommon<_Da>;

  using _unit_storage_type = typename _Da::_unit_storage_type;
  static_assert(sizeof(_unit_storage_type) == 1, "Invalid unit storage type!");
  using _unit_storage_pointer = typename _Da::_const_unit_storage_pointer;

  using typename _common::_index_type;
  using typename _common::_char_type;

 private:
  _unit_storage_pointer pointer_;

  friend typename _Da::_self;

  template<size_t Offset>
  _index_type _index() const { return *reinterpret_cast<const _index_type *>(pointer_ + Offset); }

 public:
  explicit _DoubleArrayUnitBasicConstReference(const _DoubleArrayUnitBasicReference<_Da> &x) : pointer_(x.pointer_) {}

  _index_type check() const { return _index<_common::kCheckInsets>() bitand _common::kIndexMask; }

  _index_type target() const { return _index<_common::kBaseInsets>() bitand _common::kIndexMask; }

  _index_type base() const { return target(); }

  bool has_label() const { return *(pointer_ + _common::kLabelFlagInsets) bitand 0x40; }

  bool label_is_suffix() const { return *(pointer_ + _common::kSuffixFlagInsets) bitand 0x20; }

  _index_type pool_index() const {
    assert(has_label());
    return base();
  }

  _char_type child() const { return *(pointer_ + _common::kChildInsets); }

  _char_type sibling() const { return *(pointer_ + _common::kSiblingInsets); }

  bool base_empty() const { return *(pointer_ + _common::kEdgeFlagInsets) bitand 0x80; }

  bool check_empty() const { return *(pointer_ + _common::kEmptyFlagInsets) bitand 0x80; }

  bool is_leaf() const {return base_empty() or (has_label() and label_is_suffix());}

  _index_type pred() const {
    assert(_index<_common::kPredInsets>() bitand _common::kEmptyFlag);
    return _index<_common::kPredInsets>() bitand _common::kIndexMask;
  }

  _index_type succ() const {
    assert(_index<_common::kSuccInsets>() bitand _common::kEmptyFlag);
    return _index<_common::kSuccInsets>() bitand _common::kIndexMask;
  }

 private:
  explicit _DoubleArrayUnitBasicConstReference(_unit_storage_pointer pointer) : pointer_(pointer) {}

};


// MARK: compact unit

//  Double-array unit implementation
//             ---------------------------------------------------------------------------------------------------------------------------------------------
//    Enabled  |             | Check[Byte] | Sibling[Byte] | Target(base or label-idx)[Word] | flag-Suffix[bit] | flag-Label[bit] | flag-Target-empty[bit] |
//             | Child[Byte] -------------------------------------------------------------------------------------------------------------------------------
//    Disabled |             | Pred[Byte]  | Succ[Byte]    |                                                                      |           1            |
//             ---------------------------------------------------------------------------------------------------------------------------------------------
//             |
//          pointer_
//
template <class _Ctnr>
class _DoubleArrayUnitLetterCheckReferenceCommon {
 public:
  using _container = _Ctnr;
  using _unit_storage_type = typename _container::_unit_storage_type;
  static_assert(sizeof(_unit_storage_type) == 1, "Invalid unit storage type!");
  using _index_type = typename _Ctnr::index_type;
  using _char_type = typename _Ctnr::char_type;
  using _inset_type = typename _Ctnr::inset_type;

  static constexpr size_t kIndexBytes = sizeof(_index_type);
  static constexpr _index_type kUpperBit = 1ull << (kIndexBytes*8-1);
  static constexpr _index_type kSecondBit = kUpperBit >> 1;
  static constexpr _index_type kThirdBit = kUpperBit >> 2;
  static constexpr _index_type kEmptyFlag = kUpperBit;
  static constexpr _index_type kLabelFlag = kSecondBit;
  static constexpr _index_type kSuffixFlag = kThirdBit;
  static constexpr _index_type kIndexMask = kThirdBit - 1;

  static constexpr auto kEmptyChar = 0xff;

  static constexpr size_t kChildInsets = 0;
  static constexpr size_t kCheckInsets = 1;
  static constexpr size_t kSiblingInsets = 2;
  static constexpr size_t kTargetInsets = 3;
  static constexpr size_t kFlagsInsets = kTargetInsets + kIndexBytes - 1;

  static constexpr size_t kPredInsets = 1;
  static constexpr size_t kSuccInsets = 2;
  static constexpr size_t kUnitSize = kTargetInsets + kIndexBytes;
};


template <class _Ctnr>
class _DoubleArrayUnitLetterCheckConstReference;

template <class _Cntr>
class _DoubleArrayUnitLetterCheckReference : public _DoubleArrayUnitLetterCheckReferenceCommon<_Cntr> {
  using _common = _DoubleArrayUnitLetterCheckReferenceCommon<_Cntr>;
  using typename _common::_container;

  using _unit_storage_type = typename _common::_unit_storage_type;
  using _unit_storage_pointer = typename _container::_unit_storage_pointer;

  using typename _common::_index_type;
  using typename _common::_char_type;
  using typename _common::_inset_type;

 private:
  _unit_storage_pointer pointer_;

  friend typename _container::_self;
  friend class _DoubleArrayUnitLetterCheckConstReference<_Cntr>;

  template <size_t Offset>
  _char_type _char() const {return *(_char_type*)(pointer_ + Offset);}

  template <size_t Offset>
  _char_type& _char() {return *(_char_type*)(pointer_ + Offset);}

  template <size_t Offset>
  _index_type _index() const {return *reinterpret_cast<const _index_type*>(pointer_ + Offset);}

  template <size_t Offset>
  _index_type& _index() {return *reinterpret_cast<_index_type*>(pointer_ + Offset);}

  _unit_storage_type _flags() const {return *(pointer_ + _common::kFlagsInsets);}

 public:
  _char_type child() const {return _char<_common::kChildInsets>();}

  void set_child(_char_type new_child) {
    _char<_common::kChildInsets>() = new_child;
  }

  _char_type check() const {return _char<_common::kCheckInsets>();}

  void set_check(_char_type new_check) {
    _char<_common::kCheckInsets>() = new_check;
  }

  _char_type sibling() const {return _char<_common::kSiblingInsets>();}

  void set_sibling(_char_type new_sibling) {
    _char<_common::kSiblingInsets>() = new_sibling;
  }

  _index_type target() const {return _index<_common::kTargetInsets>() bitand _common::kIndexMask;}

  _index_type base() const {assert(not has_label()); return target();}

  void set_base(_index_type new_base) {
    _index<_common::kTargetInsets>() = new_base bitand _common::kIndexMask;
  }

  bool has_label() const {return _flags() bitand 0x40;}

  bool label_is_suffix() const {return _flags() bitand 0x20;}

  _index_type pool_index() const {assert(has_label()); return target();}

  void set_pool_index(_index_type new_pool_index, bool label_is_suffix) {
    _index<_common::kTargetInsets>() = ((new_pool_index bitand _common::kIndexMask) bitor
        _common::kLabelFlag bitor
        (label_is_suffix ? _common::kSuffixFlag : 0));
  }

  bool target_empty() const {return _flags() bitand 0x80;}

  bool is_leaf() const {return target_empty() or (has_label() and label_is_suffix());}

  bool check_empty() const {return check() == _common::kEmptyChar;}

  _inset_type pred() const {return (_inset_type)*(pointer_+_common::kPredInsets);}

  void set_pred(_inset_type new_pred) {
    *(pointer_+_common::kPredInsets) = new_pred;
  }

  _inset_type succ() const {return (_inset_type)*(pointer_+_common::kSuccInsets);}

  void set_succ(_inset_type new_succ) {
    *(pointer_+_common::kSuccInsets) = new_succ;
  }

  void clean_with_link(uint8_t pred, uint8_t succ) {
    set_child(_common::kEmptyChar);
    init_disabled_unit(pred, succ);
  }

  void init_disabled_unit(uint8_t pred, uint8_t succ) {
    set_pred(pred);
    set_succ(succ);
    _index<_common::kTargetInsets>() = _common::kEmptyFlag;
  }

  void init_unit() {
    auto i = _common::kChildInsets+1;
    for (; i <= _common::kSiblingInsets; i++)
      *(pointer_+i) = _common::kEmptyChar;
    _index<_common::kTargetInsets>() = _common::kEmptyFlag;
  }

  void init_unit(_char_type check, _char_type sibling) {
    set_check(check);
    set_sibling(sibling);
    _index<_common::kTargetInsets>() = _common::kEmptyFlag;
  }

 private:
  explicit _DoubleArrayUnitLetterCheckReference(_unit_storage_pointer pointer) : pointer_(pointer) {}

};


template <class _Ctnr>
class _DoubleArrayUnitLetterCheckConstReference : public _DoubleArrayUnitLetterCheckReferenceCommon<_Ctnr> {
  using _common = _DoubleArrayUnitLetterCheckReferenceCommon<_Ctnr>;
  using typename _common::_container;

  using _unit_storage_type = typename _common::_unit_storage_type;
  using _unit_storage_pointer = typename _container::_const_unit_storage_pointer;

  using typename _common::_index_type;
  using typename _common::_char_type;

 private:
  _unit_storage_pointer pointer_;

  friend typename _container::_self;

  template <size_t Offset>
  _char_type _char() const {return *(_char_type*)(pointer_ + Offset);}

  template <size_t Offset>
  _index_type _index() const {return *reinterpret_cast<const _index_type*>(pointer_ + Offset);}

  _unit_storage_type _flags() const {return *(pointer_ + _common::kFlagsInsets);}

 public:
  explicit _DoubleArrayUnitLetterCheckConstReference(const _DoubleArrayUnitLetterCheckReference<_Ctnr>& x) : pointer_(x.pointer_) {}

  _char_type child() const {return _char<_common::kChildInsets>();}

  _char_type check() const {return _char<_common::kCheckInsets>();}

  _char_type sibling() const {return _char<_common::kSiblingInsets>();}

  bool target_empty() const {return _flags() bitand 0x80;}

  _index_type target() const {return _index<_common::kTargetInsets>() bitand _common::kIndexMask;}

  _index_type base() const {assert(not has_label()); return target();}

  bool has_label() const {return _flags() bitand 0x40;}

  bool label_is_suffix() const {return _flags() bitand 0x20;}

  _index_type pool_index() const {assert(has_label()); return target();}

  bool is_leaf() const {return target_empty() or (has_label() and label_is_suffix());}

  bool check_empty() const {return check() == _common::kEmptyChar;}

  uint8_t pred() const {return *(uint8_t*)(pointer_+_common::kPredInsets);}

  uint8_t succ() const {return *(uint8_t*)(pointer_+_common::kSuccInsets);}

 private:
  explicit _DoubleArrayUnitLetterCheckConstReference(_unit_storage_pointer pointer) : pointer_(pointer) {}

};


struct _DoubleArrayUnitReference {
  template <class _Ctnr>
  struct Basic {
    using type = _DoubleArrayUnitBasicReference<_Ctnr>;
    using const_type = _DoubleArrayUnitBasicConstReference<_Ctnr>;
  };
  template <class _Ctnr>
  struct LetterCheck {
    using type = _DoubleArrayUnitLetterCheckReference<_Ctnr>;
    using const_type = _DoubleArrayUnitLetterCheckConstReference<_Ctnr>;
  };
};


template <typename IndexType, typename CharType, typename InsetType, bool LetterCheck>
class _DoubleArrayUnitContainer {
 public:
  using _self = _DoubleArrayUnitContainer;
  using _unit_storage_type = uint8_t;
  using _unit_storage_pointer = _unit_storage_type*;
  using _const_unit_storage_pointer = const _unit_storage_type*;

  using index_type = IndexType;
  using char_type = CharType;
  using inset_type = InsetType;

  using _unit_reference_type = std::conditional_t<not LetterCheck, _DoubleArrayUnitReference::Basic<_self>, _DoubleArrayUnitReference::LetterCheck<_self>>;
  using _unit_reference = typename _unit_reference_type::type;
  using _const_unit_reference = typename _unit_reference_type::const_type;

  static constexpr auto kUnitSize = _unit_reference::kUnitSize;

 private:
  std::vector<_unit_storage_type> container_;

 public:
  _unit_reference operator[](size_t index) {
    return _unit_reference(container_.data() + (kUnitSize * index));
  }

  _const_unit_reference operator[](size_t index) const {
    return _const_unit_reference(container_.data() + (kUnitSize * index));
  }

  size_t size() const {return container_.size() / kUnitSize;}

  void resize(size_t new_size) {
    container_.resize(kUnitSize * new_size);
  }

  size_t size_in_bytes() const {return size_vec(container_);}

  void write(std::ostream& os) const {
    write_vec(container_, os);
  }

  void read(std::istream& is) {
    read_vec(is, container_);
  }

};

}

#endif /* DoubleArrayUnit_hpp */
