#ifndef INTEGRITY_H
#define INTEGRITY_H

/* Verify that engine source files have not been modified.
 * Reads .engine_checksums and compares SHA-256 hashes.
 * Returns 1 if all files are intact, 0 if tampered. */
int verify_engine_integrity(void);

#endif
