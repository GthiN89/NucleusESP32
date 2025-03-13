#ifndef PDOL_H
#define PDOL_H

#include <stdint.h>

#define MAX_VALUE_SIZE 20  // Maximum size based on the largest value array in use

typedef struct {
  uint16_t tag;
  uint8_t value[MAX_VALUE_SIZE];
  uint8_t value_size;
} PDOLValue;

// Terminal parameters requested by the card:
const PDOLValue pdol_term_info = {0x9F59, {0xC8, 0x80, 0x00}, 3};
const PDOLValue pdol_term_type = {0x9F5A, {0x00}, 1};
const PDOLValue pdol_merchant_type = {0x9F58, {0x01}, 1};
const PDOLValue pdol_term_trans_qualifies = {0x9F66, {0x79, 0x00, 0x40, 0x80}, 4};
const PDOLValue pdol_addtnl_term_qualifies = {0x9F40, {0x79, 0x00, 0x40, 0x80}, 4};
const PDOLValue pdol_amount_authorise = {0x9F02, {0x00, 0x00, 0x00, 0x10, 0x00, 0x00}, 6};
const PDOLValue pdol_amount = {0x9F03, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 6};
const PDOLValue pdol_country_code = {0x9F1A, {0x01, 0x24}, 2};
const PDOLValue pdol_currency_code = {0x5F2A, {0x01, 0x24}, 2};
const PDOLValue pdol_term_verification = {0x95, {0x00, 0x00, 0x00, 0x00, 0x00}, 5};
const PDOLValue pdol_transaction_date = {0x9A, {0x19, 0x01, 0x01}, 3};
const PDOLValue pdol_transaction_type = {0x9C, {0x00}, 1};
const PDOLValue pdol_transaction_cert = {0x98, {0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                         0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 20};
const PDOLValue pdol_unpredict_number = {0x9F37, {0x82, 0x3D, 0xDE, 0x7A}, 4};

const PDOLValue* const pdol_values[] = {
  &pdol_term_info,
  &pdol_term_type,
  &pdol_merchant_type,
  &pdol_term_trans_qualifies,
  &pdol_addtnl_term_qualifies,
  &pdol_amount_authorise,
  &pdol_amount,
  &pdol_country_code,
  &pdol_currency_code,
  &pdol_term_verification,
  &pdol_transaction_date,
  &pdol_transaction_type,
  &pdol_transaction_cert,
  &pdol_unpredict_number,
};

#endif
