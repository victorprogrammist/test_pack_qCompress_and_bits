
#ifndef PACK_TYPES_H
#define PACK_TYPES_H

#include "../tls/tls.h"
#include "../process/proc_types.h"

template <class T> struct Pack {
    static void pack(PackBits& bits, const T& v) { T::pack(bits, v); }
    static void unpack(T& v, const PackBits& bits) { T::unpack(v, bits); }
};

template <class T> void use_pack_bits(PackBits& bits, const T& v) {
    Pack<T>::pack(bits, v); }

template <class T> T& use_unpack_bits(T& v, const PackBits& b) {
    Pack<T>::unpack(v, b);
    return v; }

//************************************************************************************

template <class T>
struct Pack<std::vector<T>> {

    // для целей сохранения в базу в векторе
    //   д.б. хотя бы одни элемент,
    //   поскольку иначе это сохраняется как пустой MapId

    static void pack(PackBits& b, const std::vector<T>& v) {
        int sz = v.size();
        assert( sz > 0 );
        b.pack_count_1(sz);
        for (int i = 0; i < sz; ++i)
            use_pack_bits(b, v.at(i));

        //=== ТЕСТ СРАВНЕНИЯ УПАКОВКИ qCompress ===
        if (b.size_bytes() <= 8) return;

        QByteArray bt_raw;
        for (int i = 0; i < sz; ++i)
            bt_raw.append(QByteArray((char*)&v.at(i), sizeof(T)));

        QByteArray bt = b.get_bytes();
        qDebug() << "raw" << bt_raw.size()
                 << "zip_raw" << qCompress(bt_raw, 9).size()
                 << "my_conv_to_bits" << bt.size()
                 << "zip_my_conv_to_bits" << qCompress(bt, 9).size();
        uint a = 5;
    }

    static void unpack(std::vector<T>& v, const PackBits& b) {
        assert( b.empty() == false && v.empty() );
        int sz = b.unpack_count_1();
        for (int i = 0; i < sz; ++i) {
            T temp;
            v.push_back(use_unpack_bits(temp, b));
        }
    }
};

//************************************************************************************

template <>
struct Pack<std::vector<IdPos>> {

    // для целей сохранения в базу в векторе
    //   д.б. хотя бы одни элемент,
    //   поскольку иначе это сохраняется как пустой MapId

    static void pack(PackBits& b, const std::vector<IdPos>& v) {
        uint sz = v.size();
        assert( sz > 0 );
        b.pack_count_1(sz);

        u64 prev_val = 0;

        uint ma_c_bits = 0;
        for (uint i = 0; i < sz; ++i) {
            u64 val = v.at(i).num();

            assert( val > prev_val );
            u64 delta = val - prev_val;
            prev_val = val;

            ma_c_bits = std::max(ma_c_bits, calc_count_bits(delta));
        }

        b.pack_count_1(ma_c_bits);

        prev_val = 0;

        for (uint i = 0; i < sz; ++i) {

            u64 val = v.at(i).num();
            u64 delta = val - prev_val;
            prev_val = val;

            b.add_as_u64(delta, ma_c_bits);
        }
    }

    static void unpack(std::vector<IdPos>& v, const PackBits& b) {
        assert( b.empty() == false && v.empty() );
        uint sz = b.unpack_count_1();
        uint ma_c_bits = b.unpack_count_1();

        u64 prev_val = 0;

        for (uint i = 0; i < sz; ++i) {
            u64 delta = b.get_as_u64(ma_c_bits);
            u64 val = prev_val + delta;
            prev_val = val;

            v.push_back(IdPos::raw(val));
        }
    }
};

//************************************************************************************

template <> struct Pack<u32> {
    static void pack(PackBits& bits, const u32& v) { bits.pack_count_0(v); }
    static void unpack(u32& v, const PackBits& bits) { v = bits.unpack_count_0(); }
};

template <> struct Pack<u64> {
    static void pack(PackBits& bits, const u64& v) { bits.pack_u64(v, 4, 1); }
    static void unpack(u64& v, const PackBits& bits) { v = bits.unpack_u64(4, 1); }
};

template <> struct Pack<IdPtrn> {
    static void pack(PackBits& bits, const IdPtrn& v) { bits.pack_u64(v.num(), 7, 0); }
    static void unpack(IdPtrn& v, const PackBits& bits) { v = IdPtrn::raw(bits.unpack_u64(7, 0)); }
};

template <> struct Pack<IdScan> {
    static void pack(PackBits& bits, const IdScan& v) { bits.pack_u64(v.num(), 7, 0); }
    static void unpack(IdScan& v, const PackBits& bits) { v = IdScan::raw(bits.unpack_u64(7, 0)); }
};

template <> struct Pack<IdVal> {
    static void pack(PackBits& bits, const IdVal& v) { bits.pack_u64(v.num(), 3, 1); }
    static void unpack(IdVal& v, const PackBits& bits) { v = IdVal::raw(bits.unpack_u64(3, 1)); }
};

template <> struct Pack<MapId> {
    static void pack(PackBits& bits, const MapId& v) { bits.pack_u64(v.num(), 4, 1); }
    static void unpack(MapId& v, const PackBits& bits) { v = MapId::raw(bits.unpack_u64(4, 1)); }
};

template <> struct Pack<IdPos> {
    static void pack(PackBits& bits, const IdPos& v) { bits.pack_u64(v.num(), 4, 1); }
    static void unpack(IdPos& v, const PackBits& bits) { v = IdPos::raw(bits.unpack_u64(4, 1)); }
};

template <class T, class I> struct Pack<tmpl_SoftId<T,I>> {
    static void pack(PackBits& bits, const tmpl_SoftId<T,I>& v) { use_pack_bits(bits, v.id()); }
    static void unpack(tmpl_SoftId<T,I>& v, const PackBits& bits) { I id; v = use_unpack_bits(id, bits); }
};

//****************************************************
#endif // PACK_TYPES_H
