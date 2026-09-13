#if defined(_WIN32)
// Windows: MSVC als Host-Compiler kennt kein __int128. Dieselbe Rechnung mit 64 Bit und Uebertrag.
#ifndef CYC_PORTABLE_CARRY
#define CYC_PORTABLE_CARRY
// a + b + cin (cin 0 oder 1), Uebertrag nach *cout
static __host__ __device__ __forceinline__ uint64_t cyc_adc64(uint64_t a, uint64_t b, uint64_t cin, uint64_t* cout) {
    uint64_t s = a + b;
    uint64_t c = (s < a) ? 1ull : 0ull;
    uint64_t r = s + cin;
    c |= (r < s) ? 1ull : 0ull;
    *cout = c;
    return r;
}
// a - b - bin (bin 0 oder 1), Borgen nach *bout
static __host__ __device__ __forceinline__ uint64_t cyc_sbb64(uint64_t a, uint64_t b, uint64_t bin, uint64_t* bout) {
    uint64_t d = a - b;
    uint64_t o = (a < b) ? 1ull : 0ull;
    uint64_t r = d - bin;
    o |= (d < bin) ? 1ull : 0ull;
    *bout = o;
    return r;
}
#endif

__host__ __forceinline__ void add256_u64(const uint64_t a[4], uint64_t b, uint64_t out[4]) {
    uint64_t carry = 0;
    out[0] = cyc_adc64(a[0], b, 0ull, &carry);
    for (int i = 1; i < 4; ++i) out[i] = cyc_adc64(a[i], 0ull, carry, &carry);
}

__host__ __forceinline__ void add256(const uint64_t a[4], const uint64_t b[4], uint64_t out[4]) {
    uint64_t carry = 0;
    for (int i = 0; i < 4; ++i) out[i] = cyc_adc64(a[i], b[i], carry, &carry);
}

__host__ __forceinline__ void sub256(const uint64_t a[4], const uint64_t b[4], uint64_t out[4]) {
    uint64_t borrow = 0;
    for (int i = 0; i < 4; ++i) {
        uint64_t bi = b[i] + borrow;
        if (a[i] < bi) {
            out[i] = a[i] - bi;   // entspricht (2^64 + a[i]) - bi
            borrow = 1;
        } else {
            out[i] = a[i] - bi;
            borrow = 0;
        }
    }
}

__host__ __forceinline__ void inc256(uint64_t a[4], uint64_t inc) {
    uint64_t carry = 0;
    a[0] = cyc_adc64(a[0], inc, 0ull, &carry);
    for (int i = 1; i < 4 && carry; ++i) a[i] = cyc_adc64(a[i], 0ull, carry, &carry);
}

// Nur beim Start aufgerufen (Aufteilung des Bereichs): bitweise Division (remainder:value[i]) / divisor
__host__ void divmod_256_by_u64(const uint64_t value[4], uint64_t divisor, uint64_t quotient[4], uint64_t &remainder) {
    remainder = 0;
    for (int i = 3; i >= 0; --i) {
        uint64_t rem = remainder, q = 0, v = value[i];
        for (int bit = 63; bit >= 0; --bit) {
            uint64_t top = rem >> 63;
            rem = (rem << 1) | ((v >> bit) & 1ull);
            q <<= 1;
            if (top || rem >= divisor) { rem -= divisor; q |= 1ull; }
        }
        quotient[i] = q;
        remainder = rem;
    }
}
#else
__host__ __forceinline__ void add256_u64(const uint64_t a[4], uint64_t b, uint64_t out[4]) {
    __uint128_t sum = (__uint128_t)a[0] + b;
    out[0] = (uint64_t)sum;
    uint64_t carry = (uint64_t)(sum >> 64);
    for (int i = 1; i < 4; ++i) {
        sum = (__uint128_t)a[i] + carry;
        out[i] = (uint64_t)sum;
        carry = (uint64_t)(sum >> 64);
    }
}

__host__ __forceinline__ void add256(const uint64_t a[4], const uint64_t b[4], uint64_t out[4]) {
    __uint128_t carry = 0;
    for (int i = 0; i < 4; ++i) {
        __uint128_t s = (__uint128_t)a[i] + b[i] + carry;
        out[i] = (uint64_t)s;
        carry = s >> 64;
    }
}

__host__ __forceinline__ void sub256(const uint64_t a[4], const uint64_t b[4], uint64_t out[4]) {
    uint64_t borrow = 0;
    for (int i = 0; i < 4; ++i) {
        uint64_t bi = b[i] + borrow;
        if (a[i] < bi) {
            out[i] = (uint64_t)(((__uint128_t(1) << 64) + a[i]) - bi);
            borrow = 1;
        } else {
            out[i] = a[i] - bi;
            borrow = 0;
        }
    }
}

__host__ __forceinline__ void inc256(uint64_t a[4], uint64_t inc) {
    __uint128_t cur = (__uint128_t)a[0] + inc;
    a[0] = (uint64_t)cur;
    uint64_t carry = (uint64_t)(cur >> 64);
    for (int i = 1; i < 4 && carry; ++i) {
        cur = (__uint128_t)a[i] + carry;
        a[i] = (uint64_t)cur;
        carry = (uint64_t)(cur >> 64);
    }
}

__host__ void divmod_256_by_u64(const uint64_t value[4], uint64_t divisor, uint64_t quotient[4], uint64_t &remainder) {
    remainder = 0;
    for (int i = 3; i >= 0; --i) {
        __uint128_t cur = (__uint128_t(remainder) << 64) | value[i];
        quotient[i] = (uint64_t)(cur / divisor);
        remainder = (uint64_t)(cur % divisor);
    }
}
#endif

bool hexToLE64(const std::string& h_in, uint64_t w[4]) {
    std::string h = h_in;
    if (h.size() >= 2 && (h[0] == '0') && (h[1] == 'x' || h[1] == 'X')) h = h.substr(2);
    if (h.size() > 64) return false;
    if (h.size() < 64) h = std::string(64 - h.size(), '0') + h;
    if (h.size() != 64) return false;
    for (int i = 0; i < 4; ++i) {
        std::string part = h.substr(i * 16, 16);
        w[3 - i] = std::stoull(part, nullptr, 16);
    }
    return true;
}
bool hexToHash160(const std::string& h, uint8_t hash160[20]) {
    if (h.size() != 40) return false;
    for (int i = 0; i < 20; ++i) {
        std::string byteStr = h.substr(i * 2, 2);
        hash160[i] = (uint8_t)std::stoul(byteStr, nullptr, 16);
    }
    return true;
}
std::string formatHex256(const uint64_t limbs[4]) {
    std::ostringstream oss;
    oss << std::hex << std::uppercase << std::setfill('0');
    for (int i = 3; i >= 0; --i) {
        oss << std::setw(16) << limbs[i];
    }
    return oss.str();
}

#if defined(_WIN32)
__device__ __forceinline__ void inc256_device(uint64_t a[4], uint64_t inc) {
    uint64_t carry = 0;
    a[0] = cyc_adc64(a[0], inc, 0ull, &carry);
    for (int i = 1; i < 4 && carry; ++i) a[i] = cyc_adc64(a[i], 0ull, carry, &carry);
}
#else
__device__ __forceinline__ void inc256_device(uint64_t a[4], uint64_t inc) {
    unsigned __int128 cur = (unsigned __int128)a[0] + inc;
    a[0] = (uint64_t)cur;
    uint64_t carry = (uint64_t)(cur >> 64);
    for (int i = 1; i < 4 && carry; ++i) {
        cur = (unsigned __int128)a[i] + carry;
        a[i] = (uint64_t)cur;
        carry = (uint64_t)(cur >> 64);
    }
}
#endif

static __device__ __forceinline__ uint32_t load_u32_le(const uint8_t* p) {
    return (uint32_t)p[0]
         | ((uint32_t)p[1] << 8)
         | ((uint32_t)p[2] << 16)
         | ((uint32_t)p[3] << 24);
}

static __device__ __forceinline__ bool hash160_matches_prefix_then_full(
    const uint8_t* __restrict__ h,       
    const uint8_t* __restrict__ target,  
    const uint32_t target_prefix_le)
{
    if (load_u32_le(h) != target_prefix_le) return false;
#pragma unroll
    for (int k = 4; k < 20; ++k) {
        if (h[k] != target[k]) return false;
    }
    return true;
}

__device__ __forceinline__ bool eq256_u64(const uint64_t a[4], uint64_t b) {
    return (a[0]==b) & (a[1]==0ull) & (a[2]==0ull) & (a[3]==0ull);
}

static __device__ __forceinline__ bool hash160_prefix_equals(
    const uint8_t* __restrict__ h,
    uint32_t target_prefix)
{
    return load_u32_le(h) == target_prefix;
}

// вспомогательная: a (256-бит) >= b (u64)?
__device__ __forceinline__ bool ge256_u64(const uint64_t a[4], uint64_t b) {
    if (a[3] | a[2] | a[1]) return true;  // >= 2^64
    return a[0] >= b;
}

__device__ __forceinline__ void sub256_u64_inplace(uint64_t a[4], uint64_t dec) {
    uint64_t borrow = (a[0] < dec) ? 1ull : 0ull;
    a[0] = a[0] - dec;
#pragma unroll
    for (int i = 1; i < 4; ++i) {
        uint64_t ai = a[i];
        uint64_t bi = borrow;
        a[i] = ai - bi;
        borrow = (ai < bi) ? 1ull : 0ull;
        if (!borrow) break;
    }
}

__device__ __forceinline__ unsigned long long warp_reduce_add_ull(unsigned long long v) {
    unsigned mask = 0xFFFFFFFFu;
    v += __shfl_down_sync(mask, v, 16);
    v += __shfl_down_sync(mask, v, 8);
    v += __shfl_down_sync(mask, v, 4);
    v += __shfl_down_sync(mask, v, 2);
    v += __shfl_down_sync(mask, v, 1);
    return v;
}

static inline std::string human_bytes(double bytes) {
    static const char* u[]={"B","KB","MB","GB","TB","PB"};
    int k=0;
    while(bytes>=1024.0 && k<5){ bytes/=1024.0; ++k; }
    std::ostringstream o; o.setf(std::ios::fixed); o<<std::setprecision(bytes<10?2:1)<<bytes<<" "<<u[k];
    return o.str();
}

static inline long double ld_from_u256(const uint64_t v[4]) {
    return std::ldexp((long double)v[3],192) + std::ldexp((long double)v[2],128) + std::ldexp((long double)v[1],64) + (long double)v[0];
}

static inline std::string formatCompressedPubHex(const uint64_t Rx[4], const uint64_t Ry[4]) {
    uint8_t out[33];
    out[0] = (Ry[0] & 1ULL) ? 0x03 : 0x02;
    int off=1;
    for (int limb=3; limb>=0; --limb) {
        uint64_t v = Rx[limb];
        out[off+0]=(uint8_t)(v>>56); out[off+1]=(uint8_t)(v>>48);
        out[off+2]=(uint8_t)(v>>40); out[off+3]=(uint8_t)(v>>32);
        out[off+4]=(uint8_t)(v>>24); out[off+5]=(uint8_t)(v>>16);
        out[off+6]=(uint8_t)(v>> 8); out[off+7]=(uint8_t)(v>> 0);
        off+=8;
    }
    static const char* hexd="0123456789ABCDEF";
    std::string s; s.resize(66);
    for (int i=0;i<33;++i){ s[2*i]=hexd[(out[i]>>4)&0xF]; s[2*i+1]=hexd[out[i]&0xF]; }
    return s;
}


