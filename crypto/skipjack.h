/*! @file
	\brief Skipjack declarations.

	Documentation for Skipjack is available at
	http://csrc.nist.gov/encryption/skipjack/skipjack-kea.htm.

	$Id: skipjack.h,v 1.1 2003/03/30 12:42:21 m Exp $
*/

#ifndef SOSSE_SKIPJACK_H
#define SOSSE_SKIPJACK_H

#include <types.h>

#define SKIPJACK_KEY_LEN	10	//!< SKIPJACK key size.
#define SKIPJACK_BLOCK_LEN	8	//!< SKIPJACK block length.

/*! \brief Skipjack encryption function.

	This function encrypts <EM>v</EM> with <EM>k</EM> and returns the
	encrypted data in <EM>v</EM>.

	\param v Array of eight iu8 values containing the data block.
	\param k Array of ten iu8 values containing the key.
*/
void skipjack_enc( iu8* v, iu8* k );

/*! \brief Skipjack decryption function.

	This function decrypts <EM>v</EM> with <EM>k</EM> and returns the
	decrypted data in <EM>v</EM>.

	\param v Array of eight iu8 values containing the data block.
	\param k Array of ten iu8 values containing the key.
*/
void skipjack_dec( iu8* v, iu8* k );

#endif /* SOSSE_SKIPJACK_H */

