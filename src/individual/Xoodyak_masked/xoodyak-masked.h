/*
 * Copyright (C) 2020 Southern Storm Software, Pty Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef LWCRYPTO_XOODYAK_MASKED_H
#define LWCRYPTO_XOODYAK_MASKED_H

#include "aead-common.h"

/**
 * \file xoodyak-masked.h
 * \brief Masked version of the Xoodyak authenticated encryption algorithm.
 *
 * This algorithm is experimental and is not one of the NIST competition
 * submissions.  It uses a masked version of the Xoodoo permutation to
 * absorb the key and nonce in the first block, and then switches to the
 * regular Xoodoo permutation for the associated data and plaintext.
 *
 * The theory (as yet unproven) is that this construction will provide
 * some protection for the key against power analysis side channels.
 * Please let me know what I've done wrong if this theory is incorrect.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Size of the key for masked Xoodyak.
 */
#define XOODYAK_MASKED_KEY_SIZE 16

/**
 * \brief Size of the authentication tag for masked Xoodyak.
 */
#define XOODYAK_MASKED_TAG_SIZE 16

/**
 * \brief Size of the nonce for masked Xoodyak.
 */
#define XOODYAK_MASKED_NONCE_SIZE 16

/**
 * \brief Meta-information block for the masked Xoodyak cipher.
 */
extern aead_cipher_t const xoodyak_masked_cipher;

/**
 * \brief Encrypts and authenticates a packet with masked Xoodyak.
 *
 * \param c Buffer to receive the output.
 * \param clen On exit, set to the length of the output which includes
 * the ciphertext and the 16 byte authentication tag.
 * \param m Buffer that contains the plaintext message to encrypt.
 * \param mlen Length of the plaintext message in bytes.
 * \param ad Buffer that contains associated data to authenticate
 * along with the packet but which does not need to be encrypted.
 * \param adlen Length of the associated data in bytes.
 * \param nsec Secret nonce - not used by this algorithm.
 * \param npub Points to the public nonce for the packet which must
 * be 16 bytes in length.
 * \param k Points to the 16 bytes of the key to use to encrypt the packet.
 *
 * \return 0 on success, or a negative value if there was an error in
 * the parameters.
 *
 * \sa xoodyak_masked_aead_decrypt()
 */
int xoodyak_masked_aead_encrypt
    (unsigned char *c, unsigned long long *clen,
     const unsigned char *m, unsigned long long mlen,
     const unsigned char *ad, unsigned long long adlen,
     const unsigned char *nsec,
     const unsigned char *npub,
     const unsigned char *k);

/**
 * \brief Decrypts and authenticates a packet with masked Xoodyak.
 *
 * \param m Buffer to receive the plaintext message on output.
 * \param mlen Receives the length of the plaintext message on output.
 * \param nsec Secret nonce - not used by this algorithm.
 * \param c Buffer that contains the ciphertext and authentication
 * tag to decrypt.
 * \param clen Length of the input data in bytes, which includes the
 * ciphertext and the 16 byte authentication tag.
 * \param ad Buffer that contains associated data to authenticate
 * along with the packet but which does not need to be encrypted.
 * \param adlen Length of the associated data in bytes.
 * \param npub Points to the public nonce for the packet which must
 * be 16 bytes in length.
 * \param k Points to the 16 bytes of the key to use to decrypt the packet.
 *
 * \return 0 on success, -1 if the authentication tag was incorrect,
 * or some other negative number if there was an error in the parameters.
 *
 * \sa xoodyak_masked_aead_encrypt()
 */
int xoodyak_masked_aead_decrypt
    (unsigned char *m, unsigned long long *mlen,
     unsigned char *nsec,
     const unsigned char *c, unsigned long long clen,
     const unsigned char *ad, unsigned long long adlen,
     const unsigned char *npub,
     const unsigned char *k);

#ifdef __cplusplus
}
#endif

#endif
