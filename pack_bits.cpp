
#include "pack_bits.h"
//#include "tls_tools.h"

uint calc_count_bits(u64 v) {
    uint res = 0;
    for ( ; v > 0; v >>= 1) ++res;
    return res;
}

void PackBits::pack_count_1(u64 v) {
    assert( v > 0 );

    uint len = calc_count_bits(v);

    for (uint i = 1; i < len; ++i)
        append_bit(false);

    append_bit(true);

    p_append_u64(v, len-1);
}

u64  PackBits::unpack_count_1() const {

    int len = 0;
    while (get_bit()==false)
        ++len;

    return get_as_u64(len) + (u64(1) << len);
}

void PackBits::insert_bit(int pos, bool v) {

    int sz = bits.size();
    bits.resize(sz+1);

    for (int i = sz; i > pos; --i)
        bits.setBit(i, bits.at(i-1));

    bits.setBit(pos, v);
}

u64 PackBits::p_get_unpack_u64(int& pos, uint n_discrete, uint c_inc_n_discrete) const {

    if (bits.at(pos++)==false)
        return 0;

    u64 here = p_get_as_u64(pos, n_discrete);

    u64 farther = p_get_unpack_u64(pos, n_discrete+c_inc_n_discrete, c_inc_n_discrete);

    return (farther << n_discrete) + here;
}

u64 PackBits::p_get_as_u64(int& pos, int cnt_bits) const {
    assert( pos >= 0 && cnt_bits >= 0 && pos+cnt_bits <= size_bits() );

    pos += cnt_bits;

    u64 res = 0;
    for (int i = 0; i < cnt_bits; ++i) // в обратном порядке от порядка добавления
        res = (res << 1) + (u64)bits.at(pos - i - 1);

    return res;
}

void PackBits::p_append_u64(u64 v, int cnt_bits) {
    int st = bits.size();
    bits.resize(st + cnt_bits);
    for (int i = 0; i < cnt_bits; ++i) {
        bits.setBit(st+i, v & 1);
        v = v >> 1;
    }
}

void PackBits::pack_u64(u64 v, uint n_discrete, uint c_inc_n_discrete) {

    while (v > 0) {
        append_bit(true);
        p_append_u64(v, n_discrete);
        v = v >> n_discrete;
        n_discrete += c_inc_n_discrete;
    }

    append_bit(false);
}
