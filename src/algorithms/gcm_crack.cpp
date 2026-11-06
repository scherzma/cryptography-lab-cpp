#include <iostream>
#include <vector>
#include <string>
#include <tuple>
#include <set>
#include <math/Polynom.h>

#include "ddf.h"
#include "edf.h"
#include "math/Polynom.h"
#include "math/Element.h"
#include "gcm.h"
#include "ssf.h"
#include "gcm_crack.h"

// Adapted from an external contribution.


Element get_ghash(const Element& H, const std::vector<uint8_t>& associated_data,
                       const std::vector<uint8_t>& ciphertext) {

    std::array<uint8_t, 16> len_block = get_l(associated_data, ciphertext);
    std::vector<uint8_t> padded_ad = pad_array(associated_data);
    std::vector<uint8_t> padded_text = pad_array(ciphertext);


    std::vector<uint8_t> data;
    data.reserve(padded_ad.size() + padded_text.size() + len_block.size());
    data.insert(data.end(), padded_ad.begin(), padded_ad.end());
    data.insert(data.end(), padded_text.begin(), padded_text.end());
    data.insert(data.end(), len_block.begin(), len_block.end());

    __m128i auth_key = H.value;
    __m128i Y = _mm_setzero_si128();

    std::vector<__m128i> fe_blocks;
    fe_blocks.reserve(data.size() / 16);
    for(size_t i = 0; i < data.size(); i += 16) {   // MAYBE its faster with two loops.
        std::vector<uint8_t> block(data.begin() + i, data.begin() + i + 16);
        __m128i block_fe = _mm_set_epi8( // Rip performance btw.
            block[0], block[1], block[2], block[3],
            block[4], block[5], block[6], block[7],
            block[8], block[9], block[10], block[11],
            block[12], block[13], block[14], block[15]
        );
        fe_blocks.push_back(block_fe);
    }

    for(const auto& block_fe : fe_blocks) {
        Y = _mm_xor_si128(Y, block_fe);
        Y = gfmul_m128i(Y, auth_key);
    }

    return {Y};
}

// Function to create a zeroed polynomial from a GCMMessage
Polynom get_zeroed_poly(const GCMMessage& message) {

    const std::vector<uint8_t>& ciphertext = message.ciphertext;
    const std::vector<uint8_t>& ad = message.associated_data;
    const std::vector<uint8_t>& tag = message.tag;


    std::vector<Element> coefficients;

    // Add tag coefficient
    coefficients.emplace_back(Element(_mm_set_epi8( // Rip performance btw.
            tag[0], tag[1], tag[2], tag[3],
            tag[4], tag[5], tag[6], tag[7],
            tag[8], tag[9], tag[10], tag[11],
            tag[12], tag[13], tag[14], tag[15])));

    std::array<uint8_t, 16> len_block = get_l(ad, ciphertext);
    coefficients.emplace_back(Element(_mm_set_epi8( // Rip performance btw.
            len_block[0], len_block[1], len_block[2], len_block[3],
            len_block[4], len_block[5], len_block[6], len_block[7],
            len_block[8], len_block[9], len_block[10], len_block[11],
            len_block[12], len_block[13], len_block[14], len_block[15])));


    auto padded_ad = pad_array(ad);
    auto padded_ct = pad_array(ciphertext);

    // Add ciphertext coefficients
    for (int i = padded_ct.size() - 16; i >= 0; i -= 16) {
        std::vector<uint8_t> block(padded_ct.begin() + i, padded_ct.begin() + i + 16);
        coefficients.emplace_back(Element(_mm_set_epi8(
                block[0], block[1], block[2], block[3],
                block[4], block[5], block[6], block[7],
                block[8], block[9], block[10], block[11],
                block[12], block[13], block[14], block[15])));
    }

    // Add associated data coefficients
    for (int i = padded_ad.size() - 16; i >= 0; i -= 16) {
        std::vector<uint8_t> block(padded_ad.begin() + i, padded_ad.begin() + i + 16);
        coefficients.emplace_back(Element(_mm_set_epi8(
                block[0], block[1], block[2], block[3],
                block[4], block[5], block[6], block[7],
                block[8], block[9], block[10], block[11],
                block[12], block[13], block[14], block[15])));
    }

    return Polynom(coefficients);
}


// Function to find the correct H from candidates
std::tuple<Element, Element, bool> _find_correct_h(const std::vector<Element>& h_candidates,
                                                       const GCMMessage& m1,
                                                       const GCMMessage& m3) {
    for (const auto& potential_H : h_candidates) {

        // Compute GHASH for m1
        Element m1_ghash = get_ghash(potential_H, m1.associated_data, m1.ciphertext);
        Element ek0 = m1_ghash + Element(_mm_set_epi8(
            m1.tag[0], m1.tag[1], m1.tag[2], m1.tag[3],
            m1.tag[4], m1.tag[5], m1.tag[6], m1.tag[7],
            m1.tag[8], m1.tag[9], m1.tag[10], m1.tag[11],
            m1.tag[12], m1.tag[13], m1.tag[14], m1.tag[15]));

        Element m3_ghash = get_ghash(potential_H, m3.associated_data, m3.ciphertext);
        Element expected_tag = m3_ghash + ek0;

        // Compare with actual Tag of m3
        Element actual_tag(_mm_set_epi8(
            m3.tag[0], m3.tag[1], m3.tag[2], m3.tag[3],
            m3.tag[4], m3.tag[5], m3.tag[6], m3.tag[7],
            m3.tag[8], m3.tag[9], m3.tag[10], m3.tag[11],
            m3.tag[12], m3.tag[13], m3.tag[14], m3.tag[15]));
        if (expected_tag == actual_tag) {
            return std::make_tuple(potential_H, ek0, true);
        }
    }

    // If no candidate matched, return a zero FieldElement
    return std::make_tuple(Element(), Element(), false);
}



// Function to recover H using polynomial factorization
std::tuple<Element, Element> recover_h(const Polynom& F,
                                                 const GCMMessage& m1,
                                                 const GCMMessage& m3) {

    for (const auto& [f_sff, mult]  : sff(F)) {
        for (const auto& [f_ddf, degree] : ddf(f_sff)) {

            if (degree == f_ddf.degree()) {
                auto result = _find_correct_h(f_ddf.coefficients, m1, m3);
                if (std::get<2>(result)) {
                    return std::make_tuple(std::get<0>(result), std::get<1>(result));
                }
            } else {
                std::vector<Element> roots;
                for (const auto& root : edf(f_ddf, degree)) {
                    if (root.coefficients.size() == 2) {
                        roots.emplace_back(root.coefficients[0]);
                    }
                }
                auto result = _find_correct_h(roots, m1, m3);
                if (std::get<2>(result)) {
                    return std::make_tuple(std::get<0>(result), std::get<1>(result));
                }
            }
        }
    }
}



// Main GCM Crack Function
std::tuple<std::string, std::string, std::string> gcm_crack(
    std::vector<uint8_t> nonce,
    const GCMMessage& m1,
    const GCMMessage& m2,
    const GCMMessage& m3,
    const GCMMessage& forgery)
{
    // Construct zeroed polynomials
    Polynom f1 = get_zeroed_poly(m1);
    Polynom f2 = get_zeroed_poly(m2);

    // Compute F = f1 - f2
    Polynom F = f1 + f2;

    // Make F monic
    F.monic();

    // Recover H and EK(Y0)
    Element correct_H, mask;
    std::tie(correct_H, mask) = recover_h(F, m1, m3);

    // Compute GHASH for forgery
    Element forgery_ghash = get_ghash(correct_H, forgery.associated_data, forgery.ciphertext);

    // Compute forgery Tag: Tag = GHASH_forgery XOR EK(Y0)
    Element forgery_tag_field = forgery_ghash + mask;

    return std::make_tuple(forgery_tag_field.to_block(), correct_H.to_block(), mask.to_block());
}