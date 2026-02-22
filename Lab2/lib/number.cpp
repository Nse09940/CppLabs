#include "number.h"



uint239_t FromInt(uint32_t value, uint64_t shift) {
    if (shift >= 4e10) {
        shift %= (long long)4e10;
    }
    uint239_t num;
    for (int i = 34; i >= 0 && shift; i--) {
        num.data[i] += (shift % 2) * 128;
        //std::cout << num.data[i] << ' ';
        shift >>= 1;
    }
    shift = GetShift(num) % 245;
    uint8_t position = 244 - shift;
    for (int i = 244; i >= 0; --i, --position) {
        if (position < 0) position += 245;
        num.data[position / 7] += ((1 << 6) >> (position % 7)) * (value % 2);
        value >>= 1;
    }

    return num;
}

uint239_t FromString(const char* str, uint64_t shift) {
    uint239_t Xten = FromInt(10, 0);
    uint239_t ans;
    for (int i = 0; i < strlen(str); ++i) {
        uint32_t num = str[i] - '0';
        ans = ans * Xten + FromInt(num, 0);
    }
    ans = SetShift(ans, shift);
    return ans;
}

void DoShift(uint239_t& a, int shift){
    int s = GetShift(a);
    if(shift > 0){//left do
        shift %= 245;

    }else{//right do
        shift = -shift;
        shift %= 245;
        shift = 245 - shift;
    }
    uint239_t ans;
    int shift2 = shift;
    // shift = (shift + s)%245;
    // for (int i = 34; i >= 0 && shift; i--) {
    //     ans.data[i] += (shift % 2) * 128;
    //     //std::cout << num.data[i] << ' ';
    //     shift >>= 1;
    // }
    int count = 244 - shift2;
    for (int i = 244; i >= 0; i--,count--)
    {
        if(count < 0)count += 245;
        int f = 0;
        if(a.data[i/7] & ((1 << 6) >> (i % 7))){
            f = 1;
        }
        ans.data[count / 7] += ((1 << 6) >> (count % 7))* f;
    }
    a = ans;

}
uint239_t plus( const uint239_t& lhs1,  const uint239_t& rhs1){
    uint239_t lhs = lhs1, rhs = rhs1;
    //uint239_t ans = FromInt(0, GetShift(lhs) + GetShift(rhs));
    int a = GetShift(lhs), b = GetShift(rhs);
    DoShift(lhs,-a);
    DoShift(rhs,-b);
    int carry = 0;
    uint239_t ans;
    for (int i = 244; i >= 0; i--)
    {
        int ch1 = (lhs.data[i/7] >> (6-(i % 7))) & 1;
        int ch2 = (rhs.data[i/7] >> (6 - (i % 7))) & 1;
        int f = (ch1 + ch2 + carry)% 2;
        ans.data[i/7] += (f << (6 - (i % 7)));
        carry = (ch1 + ch2 + carry) /2;
    }

    int shift = (a + b)%245;
    DoShift(ans,shift);
    // for (int i = 34; i >= 0 && shift; i--) {
    //     ans.data[i] += (shift % 2) * 128;
    //     //std::cout << num.data[i] << ' ';
    //     shift >>= 1;
    // }
    return ans;
}

uint239_t operator+(const uint239_t& lhs, const uint239_t& rhs) {
    uint239_t ans = FromInt(0, GetShift(lhs) + GetShift(rhs));
    int64_t leftpos = 244 - GetShift(lhs) % (245),
            rightpos = 244 - GetShift(rhs) % (245),
            ansaposi = 244 - GetShift(ans) % (245);
    int8_t leftval=0, rightval=0, carry = 0;

    for (int i = 244; i >= 0; --i, --leftpos, --rightpos, --ansaposi) {
        if (leftpos < 0) leftpos += 245;
        if (rightpos < 0) rightpos += 245;
        if (ansaposi < 0) ansaposi += 245;
        leftval = ((lhs.data[leftpos / 7] & ((1 << 6) >> (leftpos % 7))) ? 1 : 0);
        rightval = ((rhs.data[rightpos / 7] & ((1 << 6) >> (rightpos % 7))) ? 1 : 0);
         ans.data[ansaposi / 7] += ((leftval + rightval + carry) % 2) * ((1 << 6) >> ( ansaposi % 7));
        carry = ((leftval + rightval + carry) / 2);
    }
     //shift = (shift + s)%245;
    // for (int i = 34; i >= 0 && shift; i--) {
    //     ans.data[i] += (shift % 2) * 128;
    //     //std::cout << num.data[i] << ' ';
    //     shift >>= 1;
    // }
    return ans;
}

uint239_t operator+=(uint239_t& lhs,  const uint239_t& rhs) {
    int64_t leftpos = 244 - GetShift(lhs) % (245),
            rigthpos = 244 - GetShift(rhs) % (245);
    int8_t leftval, rightval, carry = 0;
    int64_t plshift = (GetShift(lhs)  + GetShift(rhs) )% (int64_t)(4e10);
   // std::cout << plshift << std::endl;
    for (int i = 244; i >= 0; --i, --leftpos, --rigthpos) {
        if (leftpos < 0) leftpos += 245;
        if (rigthpos < 0) rigthpos += 245;
        leftval = ((lhs.data[leftpos / 7] & ((1 << 6) >> (leftpos % 7))) ? 1 : 0);
        rightval = ((rhs.data[rigthpos / 7] & ((1 << 6) >> (rigthpos % 7))) ? 1 : 0);
              uint8_t mask = 255;
              mask ^= (((1 << 6) >> (leftpos % 7)));
              lhs.data[leftpos / 7] &= mask;
        lhs.data[leftpos / 7] |= (1 << (6 - ((leftpos % 7)))) * ((leftval + rightval + carry)%2);
        carry = ((leftval + rightval + carry) / 2);
    }
    for (int i = 34; i >=0  && plshift; i--) {
        lhs.data[i] %= 128;
        lhs.data[i] += (plshift % 2) * (1 << 7);
        plshift /= 2;
    }
    uint64_t dosh = GetShift(rhs) % 245;
    while (dosh--) {
        uint8_t x1, x2 = ((lhs.data[244 / 7] & ((1 << 6) >> (244 % 7))) ? 1 : 0);
        for (int i = 243; i >= 0; --i) {
            if (i % 2 == 1) {
                x1 = ((lhs.data[i / 7] & ((1 << 6) >> (i % 7))) ? 1 : 0);
                uint8_t mask = 255;
                mask ^= (((1 << 6) >> (i % 7)));
                lhs.data[i / 7] &= mask;
                lhs.data[i / 7] |= x2 * ((1 << 6) >> (i % 7));
            } else {
                x2 = ((lhs.data[i / 7] & ((1 << 6) >> (i % 7))) ? 1 : 0);
                uint8_t mask = 255;
                mask ^= (((1 << 6) >> (i % 7)));
                lhs.data[i / 7] &= mask;
                lhs.data[i / 7] |= x1 * ((1 << 6) >> (i % 7));
                //std::cout << lhs.data[i / 7] << ' ';
            }
        }
        uint8_t mask = 255;
        mask ^= (((1 << 6) >> (244 % 7)));
        lhs.data[244 / 7] &= mask;
        lhs.data[244 / 7] |= x2 * ((1 << 6) >> (244 % 7));
    }
    return lhs;
}



uint239_t operator-(const uint239_t& lhs, const uint239_t& rhs) {
    int64_t ashift = GetShift(lhs) - GetShift(rhs);
    if (ashift < 0) ashift += 34359738368;
    uint239_t ans = FromInt(0, ashift);
    int64_t leftpos = 244 - GetShift(lhs) % (245),
            rightpos = 244 - GetShift(rhs) % (245),
            ansposit = 244 - ashift % (245);
    int8_t leftval, rightval, ansval, carry = 0;

    for (int i = 244; i >= 0; --i, --leftpos, --rightpos, --ansposit) {
        if (leftpos < 0) leftpos += 245;
        if (rightpos < 0) rightpos += 245;
        if (ansposit < 0) ansposit += 245;
        leftval = ((lhs.data[leftpos / 7] & ((1 << 6) >> (leftpos % 7))) ? 1 : 0);
        rightval = ((rhs.data[rightpos / 7] & ((1 << 6) >> (rightpos % 7))) ? 1 : 0);

        if((leftval + carry)-rightval < 0){
            ansval = 2 + (leftval + carry)-rightval;
            carry = -1;
        }else{
             ansval =  (leftval + carry)-rightval;
             carry = 0;
        }
        ans.data[ansposit / 7] += ansval * ((1 << 6) >> (ansposit % 7));
    }
    return ans;
}

uint239_t operator*(const uint239_t& lhs, const uint239_t& rhs) {
    uint239_t ans = FromInt(0, GetShift(lhs) + GetShift(rhs));
    int64_t rightpos = 244 - GetShift(rhs) % (245),
            ansposit = 244 - GetShift(ans) % (245);
    int8_t rightval;
    uint239_t helping_hand;

    for (int i = 244, power = 0; i >= 0; --i, --rightpos, --ansposit, ++power) {
        helping_hand = ClearShift(SetShift(lhs, power));
        if (rightpos < 0) rightpos += 245;
        if (ansposit < 0) ansposit += 245;
        rightval = ((rhs.data[rightpos / 7] & ((1 << 6) >> (rightpos % 7))) ? 1 : 0);
        if (rightval) ans = ans + helping_hand;
    }
    //for (int i = 34; i >= 0 && shift; i--) {
    //     ans.data[i] += (shift % 2) * 128;
    //     //std::cout << num.data[i] << ' ';
    //     shift >>= 1;
    // }
    return ans;
}

uint239_t operator/(const uint239_t& lhs, const uint239_t& rhs) {
    uint239_t lcopy = lhs;
    int64_t ashift = GetShift(lhs) - GetShift(rhs);
    if (ashift < 0) ashift += 34359738368;
    uint239_t ans;
    if (rhs > lhs) {
        ans = SetShift(ans, ashift);
        return ans;
    }
    uint8_t move = (len(lhs) - len(rhs));
    uint239_t helping_hand = ClearShift(SetShift(rhs, move));
    uint8_t ansval, ansposit;

    for (int i = 0; i < move + 1; ++i) {
        if (lcopy < helping_hand) {
            ansval = 0;
        } else {
            lcopy = lcopy - helping_hand;
            ansval = 1;
        }
        helping_hand =
            ClearShift(SetShift(helping_hand, 244));  
        ansposit = 244 - move + i;
        ans.data[ansposit / 7] += ansval * ((1 << 6) >> ( ansposit % 7));
    }
    ans = SetShift(ans, ashift);
    return ans;
}



bool operator==(const uint239_t& lhs, const uint239_t& rhs) {
    int64_t leftpos = 244 - GetShift(lhs) % 245,
            rightpos = 244 - GetShift(rhs) % 245;
    int8_t leftval = 0, rightval = 0;

    for (int i = 244; i >= 0; --i, --leftpos, --rightpos) {
        if (leftpos < 0) leftpos += 245;
        if (rightpos < 0) rightpos += 245;
        leftval = ((lhs.data[leftpos / 7] & ((1 << 6) >> (leftpos % 7))) ? 1 : 0);
        rightval = ((rhs.data[rightpos / 7] & ((1 << 6) >> (rightpos % 7))) ? 1 : 0);
        if (rightval != leftval) {
            return false;
        }
    }
    return true;
}

bool operator!=(const uint239_t& lhs, const uint239_t& rhs) { return (!(lhs == rhs)); }

bool operator>(const uint239_t& lhs, const uint239_t& rhs) {
    bool ans = false;
    int64_t leftpos = 244 - GetShift(lhs) % 245,
            rightpos = 244 - GetShift(rhs) % 245;
    int8_t leftval = 0, rightval = 0;

    for (int i = 244; i >= 0; --i, --leftpos, --rightpos) {
        if (leftpos < 0) leftpos += 245;
        if (rightpos < 0) rightpos += 245;
        leftval = ((lhs.data[leftpos / 7] & ((1 << 6) >> ( leftpos % 7))) ? 1 : 0);
        rightval = ((rhs.data[rightpos / 7] & ((1 << 6) >> ( rightpos % 7))) ? 1 : 0);
        if (leftval > rightval) {
            ans = true;
        } else if (rightval == leftval)
            ;
        else
            ans = false;
    }
    return ans;
}

bool operator<(const uint239_t& lhs, const uint239_t& rhs) { return rhs > lhs; }

std::ostream& operator<<(std::ostream& stream, const uint239_t& value) {
    for (int i = 0; i < 35; i++)
    {
       for (int j = 7; i >= 0; i--)
       {
            std::cout << (value.data[i] >>j);
       }
       
    }
    return stream;
}

uint64_t GetShift(const uint239_t& value) {
    uint64_t ans = 0;
    for (int i = 34; i > -1; i--) {
        ans = ans + (1 << (34 - i)) * ((128 & value.data[i]) ? 1 : 0);
    }
    return ans;
}

uint239_t SetShift(const uint239_t num, uint64_t shift) {
    uint239_t shifted ;
    if (shift >= 34359738368) shift %= 34359738368;
    if (shift < 0) shift = (shift % 34359738368) + 34359738368;
    for (int i = 34; i >= 0 && shift; i--, shift >>= 1) {
        shifted.data[i] += (shift & 1) * 128;
    }

    shift = GetShift(shifted);
    int64_t ansposit = 244 - shift % (245);
    int64_t pos = 244 - GetShift(num) % (245);
    int8_t value;

    for (int i = 244; i >= 0; --i, --pos, --ansposit) {
        if (pos < 0) pos += 245;
        if (ansposit < 0) ansposit += 245;
        value = ((num.data[pos / 7] & ((1 << 6) >> ( pos % 7))) ? 1 : 0);
        shifted.data[ansposit / 7] += value * ((1 << 6) >> ( ansposit % 7));
    }
    return shifted;
}

uint239_t ClearShift(const uint239_t num) {
    uint239_t ans = num;

    for (int i = 0; i < 1 + 34; ++i) {
        if (ans.data[i] >= 128) ans.data[i] -= 128;
    }
    return ans;
}

uint8_t len(const uint239_t& num) {
    int64_t pos = 244 - GetShift(num) % 245;
    uint8_t len, value;

    for (int i = 244; i >= 0; --i, --pos) {
        if (pos < 0) pos += 245;
        value = ((num.data[pos / 7] & ((1 << 6) >> ( pos % 7))) ? 1 : 0);
        if (value) len = 245 - i;
    }
    return len;
}
