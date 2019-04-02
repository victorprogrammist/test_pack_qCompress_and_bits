#ifndef PACK_BITS_H
#define PACK_BITS_H

#include <cstdint>
typedef uint64_t u64;

#include <QBitArray>

struct PackBits {

    PackBits() {}
    PackBits(const PackBits& oth) = default;
    PackBits(PackBits&& oth) : bits(std::move(oth.bits)), itr_pos(std::move(oth.itr_pos)) {}
    PackBits(u64 bytes) { p_append_u64(bytes, sizeof(bytes)*8); }
    PackBits(const char* bytes, int c) : bits(QBitArray::fromBits(bytes,c*8)) {}

    const char* get_data() const { return bits.bits(); }
    QByteArray  get_bytes() const { return QByteArray(get_data(), size_bytes()); }

    // нулевой бит будет младшим битом в результате
    u64 get_all_as_single_u64() const {
        assert( size_bytes() <= 8 );
        int pos = 0;
        return p_get_as_u64(pos, size_bits()); }

    bool eof() const { return itr_pos >= size_bits(); }
    bool bof() const { return itr_pos == 0; }
    void move_to_start() const { itr_pos = 0; }

    bool empty() const { return bits.isEmpty(); }
    int  size_bits() const { return bits.size(); }
    int  size_bytes() const { return (size_bits()+7) >> 3; }

    void pack_u64(u64 v, uint n_discrete, uint c_inc_n_discrete);
    u64  unpack_u64(uint n_discrete, uint c_inc_n_discrete) const {
        return p_get_unpack_u64(itr_pos, n_discrete, c_inc_n_discrete); }

    void insert_bit(int pos, bool v);
    void append_bit(bool v) { insert_bit(size_bits(), v); }

    bool get_bit() const { return p_get_bit(itr_pos); }
    u64  get_as_u64(int c_bits) const { return p_get_as_u64(itr_pos, c_bits); }
    void add_as_u64(u64 v, int c_bits) { p_append_u64(v, c_bits); }

    void pack_count_1(u64 v);
    u64  unpack_count_1() const;

    void pack_count_0(u64 v) { pack_count_1(v+1); }
    u64  unpack_count_0() const { return unpack_count_1()-1; }

private:
    QBitArray bits;
    mutable int itr_pos = 0;
    u64 p_get_unpack_u64(int& pos, uint n_discrete, uint c_inc_n_discrete) const;
    u64 p_get_as_u64(int& pos, int cnt_bits) const;
    bool p_get_bit(int& pos) const { return p_get_as_u64(pos, 1); }
    void p_append_u64(u64 v, int cnt_bits);
};


#endif // PACK_BITS_H
