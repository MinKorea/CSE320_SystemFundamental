#include "data.h"
#include "debug.h"
#include "csapp.h"

/*
 * Create a blob with given content and size.
 * The content is copied, rather than shared with the caller.
 * The returned blob has one reference, which becomes the caller's
 * responsibility.
 *
 * @param content  The content of the blob.
 * @param size  The size in bytes of the content.
 * @return  The new blob, which has reference count 1.
 */
BLOB *blob_create(char *content, size_t size)
{
	BLOB* new_blob = malloc(sizeof(BLOB));
	pthread_mutex_init(&new_blob->mutex, NULL);
	new_blob->refcnt = 1;
	new_blob->size = size;

	new_blob->content = strndup(content, size);
	new_blob->prefix = new_blob->content;

	debug("Create blob with content %p[%s], size %ld -> %p", content, new_blob->content, size, new_blob);
	return new_blob;
}

/*
 * Increase the reference count on a blob.
 *
 * @param bp  The blob.
 * @param why  Short phrase explaining the purpose of the increase.
 * @return  The blob pointer passed as the argument.
 */
BLOB *blob_ref(BLOB *bp, char *why)
{
	pthread_mutex_lock(&bp->mutex);

	// debug("%s", why);
	bp->refcnt++;

	pthread_mutex_unlock(&bp->mutex);

	return bp;
}

/*
 * Decrease the reference count on a blob.
 * If the reference count reaches zero, the blob is freed.
 *
 * @param bp  The blob.
 * @param why  Short phrase explaining the purpose of the decrease.
 */
void blob_unref(BLOB *bp, char *why)
{
	pthread_mutex_lock(&bp->mutex);

	// debug("%s", why);
	bp->refcnt--;

	pthread_mutex_unlock(&bp->mutex);

	if(bp->refcnt == 0)
	{
		free(bp->content);
		free(bp);
	}

}

/*
 * Compare two blobs for equality of their content.
 *
 * @param bp1  The first blob.
 * @param bp2  The second blob.
 * @return 0 if the blobs have equal content, nonzero otherwise.
 */
int blob_compare(BLOB *bp1, BLOB *bp2)
{
	if(strcmp(bp1->content, bp2->content) == 0)		return 0;
	else 											return 1;
}

/*
 * Hash function for hashing the content of a blob.
 *
 * @param bp  The blob.
 * @return  Hash of the blob.
 */
int blob_hash(BLOB *bp)
{
	int hash = 0;

	char *c = bp->content;
	// debug("bp_content: %s", c);

	int pos_weight = 1;

	while(*c != '\0')
	{
		int c_to_int = *c;
		unsigned int rand_num = c_to_int * pos_weight * 256;

		hash += rand_num;
		c++;
	}

	debug("BLOB %p hash: %d", bp, hash);

	return hash;
}

/*
 * Create a key from a blob.
 * The key inherits the caller's reference to the blob.
 *
 * @param bp  The blob.
 * @return  the newly created key.
 */
KEY *key_create(BLOB *bp)
{
	KEY *new_key = malloc(sizeof(KEY));

	new_key->hash = blob_hash(bp);
	new_key->blob = bp;

	debug("Create Key from blob %p -> %p [%s]", new_key->blob, new_key, new_key->blob->content);

	return new_key;
}

/*
 * Dispose of a key, decreasing the reference count of the contained blob.
 * A key must be disposed of only once and must not be referred to again
 * after it has been disposed.
 *
 * @param kp  The key.
 */
void key_dispose(KEY *kp)
{
	debug("Dispose of key %p[%s]", kp, kp->blob->content);
	blob_unref(kp->blob, "KEY DISPOSED");
	free(kp);
}

/*
 * Compare two keys for equality.
 *
 * @param kp1  The first key.
 * @param kp2  The second key.
 * @return  0 if the keys are equal, otherwise nonzero.
 */
int key_compare(KEY *kp1, KEY *kp2)
{
	if(kp1->hash == kp2->hash)	return 0;
	else  						return 1;
}

/*
 * Create a version of a blob for a specified creator transaction.
 * The version inherits the caller's reference to the blob.
 * The reference count of the creator transaction is increased to
 * account for the reference that is stored in the version.
 *
 * @param tp  The creator transaction.
 * @param bp  The blob.
 * @return  The newly created version.
 */
VERSION *version_create(TRANSACTION *tp, BLOB *bp)
{
	VERSION *new_version = malloc(sizeof(VERSION));

	new_version->creator = tp;
	trans_ref(new_version->creator, "VERSION CREATOR REFERENCE INCREASE");
	new_version->blob = bp;

	new_version->next = NULL;
	new_version->prev = NULL;

	return new_version;
}

/*
 * Dispose of a version, decreasing the reference count of the
 * creator transaction and contained blob.  A version must be
 * disposed of only once and must not be referred to again once
 * it has been disposed.
 *
 * @param vp  The version to be disposed.
 */
void version_dispose(VERSION *vp)
{
	debug("Dispose of version %p", vp);
	trans_unref(vp->creator, "VERSION DISPOSED - TRANSACTION UNREFERENCE");
	blob_unref(vp->blob, "VERSION DISPOSED - BLOB UNREFERENCE");

	// vp->prev->next = vp->next;
	// vp->next->prev = vp->prev;

	free(vp);
}