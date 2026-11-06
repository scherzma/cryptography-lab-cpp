// src/util/processor.h
#ifndef PROCESSOR_H
#define PROCESSOR_H


#include "action_parsers.h"
#include "OutputPrinter.h"

void add_numbers(const add_numbers_struct& arguments, OutputPrinter& output);
void subtract_numbers(const add_numbers_struct& arguments, OutputPrinter& output);
void poly2block(const poly2block_struct& arguments, OutputPrinter& output);
void block2poly(const block2poly_struct& arguments, OutputPrinter& output);
void gfmul(const gf_struct& arguments, OutputPrinter& output_printer);
void sea128_operation(const sea128_struct& arguments, OutputPrinter& output_printer);
void xex_fde_operation(const xex_struct& arguments, OutputPrinter& output_printer);
void padding_oracle_attack(const padding_oracle_struct& arguments, OutputPrinter& output_printer);
void gcm_encrypt(const gcm_crypt_struct& arguments, OutputPrinter& output_printer);
void gcm_decrypt(const gcm_crypt_struct& arguments, OutputPrinter& output_printer);
void polynomial_add(const poly_op_struct& arguments, OutputPrinter& output_printer);
void polynomial_mul(const poly_op_struct& arguments, OutputPrinter& output_printer);
void polynomial_pow(const poly_op_struct& arguments, OutputPrinter& output_printer);
void gfdiv(const gf_struct& arguments, OutputPrinter& output_printer);
void polynomial_divmod(const poly_op_struct& arguments, OutputPrinter& output_printer);
void polynomial_powmod(const poly_op_struct& arguments, OutputPrinter& output_printer);
void polynomial_sort(const poly_sort_struct& arguments, OutputPrinter& output_printer);
void polynomial_make_monic(const poly_op_struct& arguments, OutputPrinter& output_printer);
void polynomial_sqrt(const poly_op_struct& arguments, OutputPrinter& output_printer);
void polynomial_diff(const poly_op_struct& arguments, OutputPrinter& output_printer);
void polynomial_gcd(const poly_op_struct& arguments, OutputPrinter& output_printer);
void polynomial_factor_sff(const poly_factor_struct& arguments, OutputPrinter& output_printer);
void polynomial_factor_ddf(const poly_factor_struct& arguments, OutputPrinter& output_printer);
void polynomial_factor_edf(const poly_factor_struct& arguments, OutputPrinter& output_printer);
void gcm_crack_process(const gcm_crack_struct& arguments, OutputPrinter& output_printer);


#endif // PROCESSOR_H
