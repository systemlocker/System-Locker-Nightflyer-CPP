# Security policy

Report vulnerabilities privately through System Locker support. Do not attach
credentials, installation private keys, leases, or customer records to public
issues.

The application must embed initial Ed25519 public keys through a trusted
release process. This SDK does not learn trust roots from HTTP. It verifies
fixed JWS profiles and signatures before using lease or status claims, but an
offline client can continue until the signed lease expiry.
