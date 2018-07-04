//
//  SuffixArray.cpp
//  build
//
//  Created by 松本拓真 on 2018/05/01.
//

#include "SuffixArray.hpp"
#include "DACs.hpp"
#include "SACs.cpp"

using namespace sim_ds;

template class sim_ds::SuffixArray<DACs>;
template class sim_ds::SuffixArray<SACs2>;
template class sim_ds::SuffixArray<SACs4>;
template class sim_ds::SuffixArray<SACs8>;
