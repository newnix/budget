## `budget(1)` A personal finance tracker using C and SQLite3
This utility is meant to be a relatively simple personal finance tracker, giving you a means to track expenses in varios
categories that can be expanded and used to pull up ad-hoc estimates of your current available funds provided the database 
due to most savings and checking accounts not even holding pace with inflation, there's not currently a means for it to calculate interest.
If anyone else than me becomes interested in such a feature, that may be added in later

### Planned Features
 * Static binary - This would enable easier distribution across platforms with the same ABI
 * LibreSSL or similar crypto lib support for encrypting the database and potentially the contents of the database on the fly
 * Checksums to verify database integrity along with custom SQLite3 header settings
 * Threading to enable background optimization of the database on read-only operations and possibly support daemonization in the future
 * Interactive mode, most likely supported by Editline
