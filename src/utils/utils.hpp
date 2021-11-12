#pragma once
#include "common/common.h"

namespace layout::utils{

    //range_lower_bound
    struct RangeLowerBound : vector<pi>{

        static bool comp (const pi &x, const pi &y){
            if (x.second != y.second) return x.second < y.second;;
            return x.first < y.first;
        }

        void update (){
            if (!size()) return;
            sort(begin(), end(), comp);
            auto last = begin();
            for (auto it = begin() + 1; it < end(); ++it)
                if (it->first > last->second)
                    *++last = *it;
                else
                    last->second = it->second;
            resize(last - begin() + 1);
        }

        bool check (int left, int right) const{
            auto key = make_pair(left, right);
            auto p = lower_bound(cbegin(), cend(), key, comp);
            if (p != cbegin() && (p - 1)->second > key.first) return 1;
            if (p != cend() && key.second > p->first) return 1;
            return 0;
        }
    };
}
