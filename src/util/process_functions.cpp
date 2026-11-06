#include <iostream>
#include <set>

#include "process_functions.h"

#include "action_parsers.h"
#include "base64_utils.h"
#include "Block.h"
#include "OutputPrinter.h"
#include "semantic.h"
#include "algorithms/ddf.h"
#include "algorithms/edf.h"
#include "algorithms/gcm.h"
#include "algorithms/gcm_crack.h"
#include "algorithms/padding_oracle.h"
#include "algorithms/sea128.h"
#include "algorithms/ssf.h"
#include "algorithms/xex_fde.h"
#include "math/Element.h"
#include "math/Polynom.h"


void add_numbers(const add_numbers_struct& arguments, OutputPrinter& output) {
    output += "\"sum\": ";
    output += arguments.number1 + arguments.number2;
}

void subtract_numbers(const add_numbers_struct& arguments, OutputPrinter& output) {
    output += "\"difference\": ";
    output += arguments.number1 - arguments.number2;
}

void poly2block(const poly2block_struct& arguments, OutputPrinter& output) {
    std::vector<uint8_t> coefficients = arguments.coefficients;
    Semantic semantic = arguments.semantic;

    Block b(coefficients, semantic);

    output += "\"block\":\"";
    output += b.get_block();
    output += "\"";
}

void block2poly(const block2poly_struct& arguments, OutputPrinter& output) {
    std::array<uint8_t, 24> block = arguments.block;
    Semantic semantic = arguments.semantic;

    Block b(block, semantic);

    output += "\"coefficients\": ";
    output += b.get_poly(); // TODO: Fix this
}

void gfmul(const gf_struct& arguments, OutputPrinter& output_printer) {
    Semantic semantic = arguments.semantic;
    std::array<uint8_t, 24> a_str = arguments.a;
    std::array<uint8_t, 24> b_str = arguments.b;

    output_printer += "\"product\": \"";
    if (semantic == Semantic::GCM) {
        std::array<uint8_t, 16> S = base64_decode_16_reversed_array(a_str);
        std::array<uint8_t, 16> X = base64_decode_16_reversed_array(b_str);
        __m128i a = _mm_loadu_si128(reinterpret_cast<const __m128i*>(S.data()));
        __m128i b = _mm_loadu_si128(reinterpret_cast<const __m128i*>(X.data()));
        __m128i result = gfmul_m128i(a, b);
        std::array<uint8_t, 16> output;
        _mm_storeu_si128(reinterpret_cast<__m128i*>(output.data()), result);
        output_printer += base64_encode_16_reversed(output);
    } else {
        std::array<uint8_t, 16> S = base64_decode_16_array(a_str);
        std::array<uint8_t, 16> X = base64_decode_16_array(b_str);
        __m128i a = _mm_set_epi8(
            reverse8_table[S[0]], reverse8_table[S[1]], reverse8_table[S[2]], reverse8_table[S[3]],
            reverse8_table[S[4]], reverse8_table[S[5]], reverse8_table[S[6]], reverse8_table[S[7]],
            reverse8_table[S[8]], reverse8_table[S[9]], reverse8_table[S[10]], reverse8_table[S[11]],
            reverse8_table[S[12]], reverse8_table[S[13]], reverse8_table[S[14]], reverse8_table[S[15]]
        );
        __m128i b = _mm_set_epi8(
            reverse8_table[X[0]], reverse8_table[X[1]], reverse8_table[X[2]], reverse8_table[X[3]],
            reverse8_table[X[4]], reverse8_table[X[5]], reverse8_table[X[6]], reverse8_table[X[7]],
            reverse8_table[X[8]], reverse8_table[X[9]], reverse8_table[X[10]], reverse8_table[X[11]],
            reverse8_table[X[12]], reverse8_table[X[13]], reverse8_table[X[14]], reverse8_table[X[15]]
        );
        __m128i result = gfmul_m128i(a, b);
        std::array<uint8_t, 16> output;
        _mm_storeu_si128(reinterpret_cast<__m128i*>(output.data()), result);
        for (int i = 0; i < 16; ++i) {
            output[i] = reverse8_table[output[i]];
        }
        output_printer += base64_encode_16_reversed(output);
    }
    output_printer += "\"";
}

void sea128_operation(const sea128_struct& arguments, OutputPrinter& output_printer){
    bool encrypt = arguments.encrypt;
    std::array<uint8_t, 24> key_str = arguments.key;
    std::vector<uint8_t> input_str = arguments.input;
    std::array<uint8_t, 16> key = base64_decode_16_array(key_str);
    std::vector<uint8_t> input = base64_decode_vector(input_str);
    std::vector<uint8_t> result;

    output_printer += "\"output\": \"";
    if (encrypt) {
        std::vector<uint8_t> encrypted = encrypt_sea(key, input);
        output_printer += base64_encode(encrypted);
    } else {
        std::vector<uint8_t> decrypted = decrypt_sea(key, input);
        output_printer += base64_encode(decrypted);
    }
    output_printer += "\"";
}

void xex_fde_operation(const xex_struct& arguments, OutputPrinter& output_printer){
    bool encrypt = arguments.encrypt;
    std::vector<uint8_t> key_str = arguments.key;
    std::vector<uint8_t> tweak_str = arguments.tweak;
    std::vector<uint8_t> input_str = arguments.input;

    // Decode base64
    std::vector<uint8_t> key = base64_decode_vector(key_str);
    std::vector<uint8_t> tweak = base64_decode_vector(tweak_str);
    std::vector<uint8_t> input = base64_decode_vector(input_str);

    std::vector<uint8_t> result;
    if (encrypt){
        result = encrypt_fde(key, tweak, input);
    } else{
        result = decrypt_fde(key, tweak, input);
    }

    std::string result_b64 = base64_encode(result);

    output_printer += "\"output\": \"";
    output_printer += result_b64;
    output_printer += "\"";
}

void padding_oracle_attack(const padding_oracle_struct& arguments, OutputPrinter& output_printer){
    std::vector<uint8_t> hostname_vector = arguments.hostname;
    std::string hostname;
    hostname = std::string(hostname_vector.begin(), hostname_vector.end());

    int port = arguments.port;
    std::array<uint8_t, 24> iv_str = arguments.iv;
    std::vector<uint8_t> ciphertext_str = arguments.ciphertext;

    std::array<uint8_t, 16> iv = base64_decode_16_array(iv_str);
    std::vector<uint8_t> ciphertext = base64_decode_vector(ciphertext_str);

    std::vector<uint8_t> plaintext = padding_attack(hostname, port, iv, ciphertext);

    std::string plaintext_b64 = base64_encode(plaintext);
    output_printer += "\"plaintext\": \"";
    output_printer += plaintext_b64;
    output_printer += "\"";
}

void gcm_crypt_process(const gcm_crypt_struct& arguments, bool encrypt, OutputPrinter& output_printer){
    bool aes = arguments.aes;
    std::vector<uint8_t> nonce_str = arguments.nonce;
    std::array<uint8_t, 24> key_str = arguments.key;
    std::vector<uint8_t> ad_str = arguments.ad;
    std::vector<uint8_t> text_str = arguments.text;

    std::array<uint8_t, 16> key = base64_decode_16_array(key_str);
    std::vector<uint8_t> ad = base64_decode_vector(ad_str);
    std::vector<uint8_t> nonce = base64_decode_vector(nonce_str);
    std::vector<uint8_t> text = base64_decode_vector(text_str);

    if(encrypt){
        auto [ciphertext, tag, L, H] = encrypt_gcm(key, text, ad, nonce, aes);
        output_printer += "\"ciphertext\": \"";
        output_printer += base64_encode(ciphertext);
        output_printer += "\", \"tag\": \"";
        output_printer += base64_encode_16(tag);
        output_printer += "\", \"L\": \"";
        output_printer += base64_encode_16(L);
        output_printer += "\", \"H\": \"";
        output_printer += base64_encode_16(H);
        output_printer += "\"";
        return;
    }
    else{
        std::string tag_str(arguments.tag.begin(), arguments.tag.end());
        auto [plaintext, auth_tag, len_block, auth_key] = decrypt_gcm(key, text, ad, nonce, aes);
        output_printer += "\"plaintext\": \"";
        output_printer += base64_encode(plaintext);
        output_printer += "\", \"authentic\": ";
        output_printer += base64_encode_16(auth_tag) == tag_str;
        return;
    }
}

void gcm_encrypt(const gcm_crypt_struct& arguments, OutputPrinter& output_printer){
    gcm_crypt_process(arguments, true, output_printer);
}

void gcm_decrypt(const gcm_crypt_struct& arguments, OutputPrinter& output_printer){
    gcm_crypt_process(arguments, false, output_printer);
}



void polynomial_add(const poly_op_struct& arguments, OutputPrinter& output_printer){
    std::vector<std::array<uint8_t, 24>> A = arguments.A;
    std::vector<std::array<uint8_t, 24>> B = arguments.B;

    Polynom poly_A(A);
    Polynom poly_B(B);

    poly_A += poly_B;
    output_printer += "\"S\": ";
    output_printer += poly_A.repr();
}

void polynomial_mul(const poly_op_struct& arguments, OutputPrinter& output_printer){
    std::vector<std::array<uint8_t, 24>> A = arguments.A;
    std::vector<std::array<uint8_t, 24>> B = arguments.B;

    Polynom poly_A(A);
    Polynom poly_B(B);

    poly_A *= poly_B;
    output_printer += "\"P\": ";
    output_printer += poly_A.repr();
}

void polynomial_pow(const poly_op_struct& arguments, OutputPrinter& output_printer){
    std::vector<std::array<uint8_t, 24>> A = arguments.A;
    int k = arguments.k;

    Polynom poly_A(A);
    Polynom power = poly_A ^ k;
    output_printer += "\"Z\": ";
    output_printer += power.repr();
}

void gfdiv(const gf_struct& arguments, OutputPrinter& output_printer){
    std::array<uint8_t, 24> a_str = arguments.a;
    std::array<uint8_t, 24> b_str = arguments.b;

    Block a_blk(a_str, Semantic::GCM);
    Block b_blk(b_str, Semantic::GCM);

    Element a(a_blk);
    Element b(b_blk);
    Element c = a / b;

    output_printer += "\"q\": \"";
    output_printer += c.to_block();
    output_printer += "\"";
}

void polynomial_divmod(const poly_op_struct& arguments, OutputPrinter& output_printer){
    std::vector<std::array<uint8_t, 24>> A = arguments.A;
    std::vector<std::array<uint8_t, 24>> B = arguments.B;

    Polynom poly_A(A);
    Polynom poly_B(B);

    auto [Q, R] = poly_A.divide(poly_B);

    output_printer += "\"Q\": ";
    output_printer += Q.repr();
    output_printer += ", \"R\": ";
    output_printer += R.repr();
}

void polynomial_powmod(const poly_op_struct& arguments, OutputPrinter& output_printer){
    std::vector<std::array<uint8_t, 24>> A = arguments.A;
    std::vector<std::array<uint8_t, 24>> B = arguments.B;
    int k = arguments.k;

    Polynom poly_A(A);
    Polynom poly_M(B);

    Polynom Z = poly_A.powermod(poly_M, k);
    output_printer += "\"Z\": ";
    output_printer += Z.repr();
}

void polynomial_sort(const poly_sort_struct& arguments, OutputPrinter& output_printer){
    std::vector<std::vector<std::array<uint8_t, 24>>> polys_str = arguments.polys;
    std::vector<Polynom> polys;
    polys.reserve(polys_str.size());

    for (const auto& poly_str : polys_str) {
        polys.emplace_back(poly_str);
    }

    Polynom::sort(polys);

    std::vector<std::vector<std::string>> sorted_polys;
    sorted_polys.reserve(polys.size());

    for (auto&& poly : polys) {
        sorted_polys.emplace_back(std::move(poly.repr()));
    }

    output_printer += "\"sorted_polys\": ["; // TODO: Fix this
    for (auto&& poly : sorted_polys) {
        output_printer += poly;
        output_printer += ",";
    }
    output_printer.set(']');
}

void polynomial_make_monic(const poly_op_struct& arguments, OutputPrinter& output_printer){
    std::vector<std::array<uint8_t, 24>> A = arguments.A;
    Polynom poly_A(A);
    poly_A.monic();
    output_printer += "\"A*\": ";
    output_printer += poly_A.repr();
}

void polynomial_sqrt(const poly_op_struct& arguments, OutputPrinter& output_printer){
    std::vector<std::array<uint8_t, 24>> A = arguments.A;
    Polynom poly_Q(A);
    Polynom sqrt_poly = poly_Q.sqrt();
    output_printer += "\"S\": ";
    output_printer += sqrt_poly.repr();
}

void polynomial_diff(const poly_op_struct& arguments, OutputPrinter& output_printer){
    std::vector<std::array<uint8_t, 24>> A = arguments.A;
    Polynom poly_F(A);
    Polynom derivative = poly_F.diff();
    output_printer += "\"F'\":";
    output_printer += derivative.repr();
}

void polynomial_gcd(const poly_op_struct& arguments, OutputPrinter& output_printer){
    std::vector<std::array<uint8_t, 24>> A = arguments.A;
    std::vector<std::array<uint8_t, 24>> B = arguments.B;
    Polynom poly_A(A);
    Polynom poly_B(B);
    Polynom ans = poly_A.gcd(poly_B);
    output_printer += "\"G\": ";
    output_printer += ans.repr();
}


void polynomial_factor_sff(const poly_factor_struct& arguments, OutputPrinter& output_printer){
    std::vector<std::array<uint8_t, 24>> F = arguments.F;
    Polynom poly_F(F);
    std::set<std::tuple<Polynom, int>> factors = sff(poly_F);
    std::vector<std::tuple<Polynom, int>> factors_repr(factors.begin(), factors.end());

    std::sort(factors_repr.begin(), factors_repr.end(),
        [](const auto& a, const auto& b) {
            return std::get<0>(a) < std::get<0>(b);
        });

    output_printer += "\"factors\": [";
    bool first = true;
    for (const auto& [factor, degree] : factors_repr) {
        if (!first) {
            output_printer += ", ";
        }
        output_printer += "{";
        output_printer += "\"factor\": ";
        output_printer += factor.repr();
        output_printer += ", \"exponent\": ";
        output_printer += degree;
        output_printer += "}";
        first = false;
    }
    output_printer += "]";
}


void polynomial_factor_ddf(const poly_factor_struct& arguments, OutputPrinter& output_printer){
    std::vector<std::array<uint8_t, 24>> F = arguments.F;
    Polynom poly_F(F);
    std::set<std::tuple<Polynom, int>> factors = ddf(poly_F);
    std::vector<std::tuple<Polynom, int>> factors_repr(factors.begin(), factors.end());

    std::sort(factors_repr.begin(), factors_repr.end(),
        [](const auto& a, const auto& b) {
            return std::get<0>(a) < std::get<0>(b);
        });

    output_printer += "\"factors\": [";
    bool first = true;
    for (const auto& [factor, degree] : factors_repr) {
        if (!first) {
            output_printer += ", ";
        }
        output_printer += "{";
        output_printer += "\"factor\": ";
        output_printer += factor.repr();
        output_printer += ", \"degree\": ";
        output_printer += degree;
        output_printer += "}";
        first = false;
    }
    output_printer += "]";
}

void polynomial_factor_edf(const poly_factor_struct& arguments, OutputPrinter& output_printer){
    std::vector<std::array<uint8_t, 24>> F = arguments.F;
    int d = arguments.d;
    Polynom poly_F(F);

    std::vector<Polynom> polys = edf(poly_F, d);
    std::vector<Polynom> factors_polys(polys.begin(), polys.end());

    Polynom::sort(factors_polys);

    output_printer += "\"factors\": [";
    bool first = true;
    for(auto& factor : factors_polys){
        if (!first) {
            output_printer += ", ";
        }
        output_printer += factor.repr();
        first = false;
    }
    output_printer += "]";
}

void gcm_crack_process(const gcm_crack_struct& arguments, OutputPrinter& output_printer){
    std::string nonce;
    nonce = "bYfxz4zIS8NGWT55xSGy7Q==";

    GCMMessage m1 = arguments.m1;
    GCMMessage m2 = arguments.m2;
    GCMMessage m3 = arguments.m3;
    GCMMessage forgery = arguments.forgery;

    auto [forgery_tag, H, mask] = gcm_crack(base64_decode(nonce), m1, m2, m3, forgery);

    output_printer += "\"tag\": \"";
    output_printer += forgery_tag;
    output_printer += "\", \"H\": \"";
    output_printer += H;
    output_printer += "\", \"mask\": \"";
    output_printer += mask;
    output_printer += "\"";
}
