//
// Created by 松本拓真 on 2019-08-27.
//

#ifndef DOUBLEARRAYEXCEPTION_H_
#define DOUBLEARRAYEXCEPTION_H_

#include <exception>

namespace sim_ds {

template <class _Ctnr>
class _DoubleArrayExceptionSizeOver : std::exception {
 public:
  using _container = _Ctnr;
  using _index_type = typename _container::_index_type;
  static constexpr size_t kIndexMax = _container::kIndexMax;

 private:
  const char* _text;

 public:
  _DoubleArrayExceptionSizeOver() : _text((std::string("Array size is over at 'index_type' of double-array!\n")+
      "You should set new 'index_type' which can represents more large number(e.g. uint64_t)."+
      std::to_string(kIndexMax)).c_str()) {}

  const char* what() const noexcept override {
    return _text;
  }
};

}

#endif //DOUBLEARRAYEXCEPTION_H_
