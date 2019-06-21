# L2Launcher
Lineage II Launcher for private servers using DynDNS (non-static IPs)

-- 

This is a **VERY** simple and unsafe way to start Lineage II using Windows host name resolving, e.g. for playing on servers with non-static IPs.
It's a quick and dirty way, please feel free to fix stuff I might have forgotten or coded badly and create pull requests.

The executable modifies the C:\Windows\system32\drivers\etc\hosts file to append (or modify if already exists) the IP
for the Lineage II authentification server so you're able to access private Lineage II servers that don't have static IPs.
Again, please keep in mind that this is a 2-hour dirt work of a guy who never worked with Windows Sockets before. Thanks.
