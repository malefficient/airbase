RC4.h (MacOS)
it seems like brew/openssl/apple can't come to an arrangement on what to do about the system include path for openssl.
This has been an issue forever now, and I don't see any reason to hope it gets resolved. 
Until there is a 'correct' way to resolve this, i just did the following:
https://github.com/rakshasa/libtorrent/issues/150
```
cd /usr/local/include
cp -rf ../../Cellar/openssl/1.0.2l/include/openssl .
```
