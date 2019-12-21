/*
 * Copyright (C) 2019 Southern Storm Software, Pty Ltd.
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

#ifndef LWCRYPTO_AEAD_COMMON_H
#define LWCRYPTO_AEAD_COMMON_H

/**
 * \file aead-common.h
 * \brief Definitions that are common across AEAD schemes.
 *
 * AEAD stands for "Authenticated Encryption with Associated Data".
 * It is a standard API pattern for securely encrypting and
 * authenticating packets of data.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Encrypts and authenticates a packet with an AEAD scheme.
 *
 * \param c Buffer to receive the output.
 * \param clen On exit, set to the length of the output which includes
 * the ciphertext and the authentication tag.
 * \param m Buffer that contains the plaintext message to encrypt.
 * \param mlen Length of the plaintext message in bytes.
 * \param ad Buffer that contains associated data to authenticate
 * along with the packet but which does not need to be encrypted.
 * \param adlen Length of the associated data in bytes.
 * \param nsec Secret nonce - not used by this algorithm.
 * \param npub Points to the public nonce for the packet.
 * \param k Points to the key to use to encrypt the packet.
 *
 * \return 0 on success, or a negative value if there was an error in
 * the parameters.
 */
typedef int (*aead_cipher_encrypt_t)
    (unsigned char *c, unsigned long long *clen,
     const unsigned char *m, unsigned long long mlen,
     const unsigned char *ad, unsigned long long adlen,
     const unsigned char *nsec,
     const unsigned char *npub,
     const unsigned char *k);

/**
 * \brief Decrypts and authenticates a packet with an AEAD scheme.
 *
 * \param m Buffer to receive the plaintext message on output.
 * \param mlen Receives the length of the plaintext message on output.
 * \param nsec Secret nonce - not used by this algorithm.
 * \param c Buffer that contains the ciphertext and authentication
 * tag to decrypt.
 * \param clen Length of the input data in bytes, which includes the
 * ciphertext and the authentication tag.
 * \param ad Buffer that contains associated data to authenticate
 * along with the packet but which does not need to be encrypted.
 * \param adlen Length of the associated data in bytes.
 * \param npub Points to the public nonce for the packet.
 * \param k Points to the key to use to decrypt the packet.
 *
 * \return 0 on success, -1 if the authentication tag was incorrect,
 * or some other negative number if there was an error in the parameters.
 */
typedef int (*aead_cipher_decrypt_t)
    (unsigned char *m, unsigned long long *mlen,
     unsigned char *nsec,
     const unsigned char *c, unsigned long long clen,
     const unsigned char *ad, unsigned long long adlen,
     const unsigned char *npub,
     const unsigned char *k);

/**
 * \brief No special AEAD features.
 */
#define AEAD_FLAG_NONE          0x0000

/**
 * \brief The natural byte order of the AEAD cipher is little-endian.
 *
 * If this flag is not present, then the natural byte order of the
 * AEAD cipher should be assumed to be big-endian.
 *
 * The natural byte order may be useful when formatting packet sequence
 * numbers as nonces.  The application needs to know whether the sequence
 * number should be packed into the leading or trailing bytes of the nonce.
 */
#define AEAD_FLAG_LITTLE_ENDIAN 0x0001

/**
 * \brief Meta-information about an AEAD cipher.
 */
typedef struct
{
    const char *name;               /**< Name of the cipher */
    unsigned key_len;               /**< Length of the key in bytes */
    unsigned nonce_len;             /**< Length of the nonce in bytes */
    unsigned tag_len;               /**< Length of the tag in bytes */
    unsigned flags;                 /**< Flags for extra features */
    aead_cipher_encrypt_t encrypt;  /**< AEAD encryption function */
    aead_cipher_decrypt_t decrypt;  /**< AEAD decryption function */

} aead_cipher_t;

/**
 * \brief Simple encryption and authentication of a packet with an AEAD scheme.
 *
 * \param aead The aead_cipher_t meta-information block for the AEAD cipher.
 * \param ciphertext Buffer to receive the ciphertext output.
 * \param ciphertext_max Maximum number of bytes in the output buffer,
 * which must be large enough to contain the ciphertext and the
 * authentication tag
 * \param plaintext Buffer that contains the plaintext to encrypt.
 * \param plaintext_len Length of the plaintext in bytes.
 * \param seq_num The 64-bit packet sequence number, which must be
 * different for every packet.
 * \param key Points to the key to use to encrypt the packet.
 * \param key_len Length of the key in bytes.
 *
 * \return The number of bytes that were written to \a ciphertext which
 * includes the encrypted plaintext and the authentication tag.
 * Returns -1 if the parameters are invalid in some fashion.
 *
 * This function is simpler than the full AEAD API in that it dispenses
 * with the associated data and uses a simple 64-bit sequence number
 * instead of a nonce.
 *
 * It is incredibly important that the sequence number be different
 * for every packet that is encrypted under the same key.  The simplest
 * is to increment the sequence number after every packet.  The application
 * must change to a new key before overflow occurs.
 *
 * \sa aead_decrypt_packet()
 */
int aead_encrypt_packet
    (const aead_cipher_t *aead, unsigned char *ciphertext, int ciphertext_max,
     const unsigned char *plaintext, int plaintext_len,
     unsigned long long seq_num, const unsigned char *key, int key_len);

/**
 * \brief Simple decryption and authentication of a packet with an AEAD scheme.
 *
 * \param aead The aead_cipher_t meta-information block for the AEAD cipher.
 * \param plaintext Buffer to receive the ciphertext output.
 * \param plaintext_max Maximum number of bytes in the output buffer,
 * which must be at least \a ciphertext_len - tag_size where \a tag_size
 * is the size of the authentication tag for the underlying cipher.
 * \param ciphertext Buffer that contains the ciphertext to decrypt,
 * followed by the authentication tag.
 * \param ciphertext_len Length of the input buffer in bytes.
 * \param seq_num The 64-bit packet sequence number, which must be
 * different for every packet.
 * \param key Points to the key to use to decrypt the packet.
 * \param key_len Length of the key in bytes.
 *
 * \return The number of bytes that were written to \a plaintext which
 * excludes the authentication tag.  Returns -1 if the parameters
 * are invalid in some fashion or the authentication tag is invalid.
 *
 * This function is simpler than the full AEAD API in that it dispenses
 * with the associated data and uses a simple 64-bit sequence number
 * instead of a nonce.
 *
 * \sa aead_encrypt_packet()
 */
int aead_decrypt_packet
    (const aead_cipher_t *aead, unsigned char *plaintext, int plaintext_max,
     const unsigned char *ciphertext, int ciphertext_len,
     unsigned long long seq_num, const unsigned char *key, int key_len);

#ifdef __cplusplus
}
#endif

#endif