#include "sha256.hpp"
#include <windows.h>
#include <wincrypt.h>
#include <winss/windows_interface.hpp>
#include <easylogging/easylogging++.hpp>
#include <string>
#include <vector>

class ScopedHCRYPTPROV {
 private:
    HCRYPTPROV provider = NULL;

 public:
    ScopedHCRYPTPROV() {
        std::memset(&provider, 0, sizeof(HCRYPTPROV));
    }

    HCRYPTPROV Get() {
        return provider;
    }

    HCRYPTPROV* Recieve() {
        return &provider;
    }

    ~ScopedHCRYPTPROV() {
        if (provider) {
            WINDOWS.CryptReleaseContext(provider, 0);
        }
    }
};

class ScopedHCRYPTHASH {
 private:
     HCRYPTHASH hash;

 public:
    ScopedHCRYPTHASH() {
        std::memset(&hash, 0, sizeof(HCRYPTHASH));
    }

    HCRYPTHASH Get() {
        return hash;
    }

    HCRYPTPROV* Recieve() {
        return &hash;
    }

    ~ScopedHCRYPTHASH() {
        if (hash) {
            WINDOWS.CryptDestroyHash(hash);
        }
    }
};

std::string winss::SHA256::CalculateDigest(const std::string& value) {
    ScopedHCRYPTPROV provider;
    if (!WINDOWS.CryptAcquireContext(provider.Recieve(), nullptr, nullptr,
        PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        VLOG(1) << "CryptAcquireContext failed: " << WINDOWS.GetLastError();
        return value;
    }

    ScopedHCRYPTHASH hash;
    if (!WINDOWS.CryptCreateHash(provider.Get(), CALG_SHA_256, 0, 0,
        hash.Recieve())) {
        VLOG(1) << "CryptCreateHash failed: " << WINDOWS.GetLastError();
        return value;
    }

    if (!WINDOWS.CryptHashData(hash.Get(),
        const_cast<unsigned char*>(
            reinterpret_cast<const unsigned char*>(value.c_str())),
        static_cast<DWORD>(value.length()), 0)) {
        VLOG(1) << "CryptHashData failed: " << WINDOWS.GetLastError();
        return value;
    }

    DWORD hash_len = 0;
    DWORD buffer_size = sizeof(DWORD);
    if (!WINDOWS.CryptGetHashParam(hash.Get(), HP_HASHSIZE,
        reinterpret_cast<unsigned char*>(&hash_len), &buffer_size, 0)) {
        VLOG(1)
            << "CryptGetHashParam(HP_HASHSIZE) failed: "
            << WINDOWS.GetLastError();
        return value;
    }

    std::vector<unsigned char> buffer(hash_len);
    if (!WINDOWS.CryptGetHashParam(hash.Get(), HP_HASHVAL,
        reinterpret_cast<unsigned char*>(&buffer[0]), &hash_len, 0)) {
        VLOG(1)
            << "CryptGetHashParam(HP_HASHVAL) failed: "
            << WINDOWS.GetLastError();
        return value;
    }

    std::ostringstream oss;
    std::vector<unsigned char>::const_iterator it;
    for (it = buffer.begin(); it != buffer.end(); ++it) {
        oss.fill('0');
        oss.width(2);
        oss << std::hex << static_cast<const int>(*it);
    }

    return oss.str();
}
