# Security policy

Report vulnerabilities privately through System Locker support. Do not attach
credentials, installation private keys, leases, or customer records to public
issues.

The application must embed initial Ed25519 public keys through a trusted
release process. This SDK does not learn trust roots from HTTP. It verifies
fixed JWS profiles and signatures before using lease or status claims. Saved
signing keys are rebuilt only by replaying server-signed transitions from those
embedded roots, and the complete state record is signed by the installation
key.

Persistent deployments should use an OS-protected, preferably non-exportable
installation key. By default, cross-reboot offline use is accepted only when
UTC has not moved behind the installation-key-signed high-water checkpoint.
This rejects simple wall-clock rollback while preserving normal offline
restarts and time-zone changes. Software-only storage cannot distinguish every
coordinated replay of an older, correctly signed snapshot and matching clock
value. Set `requireOnlineAfterReboot` to true to require a signed online
response after each OS boot when that stricter tradeoff is appropriate. An
offline client still cannot observe server-side revocation before reconnecting.
