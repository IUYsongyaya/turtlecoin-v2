# Wallet Overview

## Seed Generation

Generate a random seed from a CSRNG source whenever possible or alternatively
another RNG source.

A seed should be 32-bytes (64 hexadecimal characters) which can be directly
translated to a mnemonic phrase.

An example of seed generation, including the mnemonic words, and seed generation UNIX timestamp is below:

```c++
#include <crypto.h>

const auto [wallet_seed, mnemonic_words, timestamp] = Crypto::generate_wallet_seed();
```

**All keys are derived from the initial seed.**

**Seeds should not be used directly as any keys.**

**The underlying cryptographic primitives are domain salted to avoid collisions
and as such, it is imperative that all wallets use the same domain salts to
ensure compatibility when deriving keys from a seed.**

## Mnemonic Phrase

### Encoding

Using the wallet seed, we can transform the wallet seed into a [mnemonic](https://en.wikipedia.org/wiki/Mnemonic) phrase.

By default the UNIX timestamp is encoded into the resulting mnemonic words.

```c++
#include <crypto.h>
#include <utilities.h>

const auto mnemonic_words = Crypto::Mnemonics::encode(wallet_seed);

const auto mnemonic_phrase = TurtleCoin::Utilities::str_join(mnemonic_words);
```

To encode a specific UNIX timestamp instead of the current UNIX timestamp, we do so like this:

```c++
#include <crypto.h>
#include <utilities.h>

const auto mnemonic_words = Crypto::Mnemonics::encode(wallet_seed, 1622683338, false);

const auto mnemonic_phrase = TurtleCoin::Utilities::str_join(mnemonic_words);
```

### Decoding

Using the mnemonic phrase, we can transform that phrase into a wallet seed.

**Note**: When decoding a mnemonic phrase, if the phrase includes the UNIX timestamp that will be returned else 0 is returned.

```c++
#include <crypto.h>
#include <utilities.h>

const std::string mnemonic_phrase = "chalk allow empower truck audit fragile focus elevator kid animal fire nerve work version bright ketchup wrestle erupt gun heart right stool earn pizza push";

const auto mnemonic_words = TurtleCoin::Utilities::str_split(mnemonic_phrase);

const auto [success, wallet_seed, timestamp] = Crypto::restore_wallet_seed(mnemonic_words);

if (success)
{
    std::cout << wallet_seed << std::endl;
}
```

Alternatively (functionally the same):

```c++
#include <crypto.h>
#include <utilities.h>

const std::string mnemonic_phrase = "chalk allow empower truck audit fragile focus elevator kid animal fire nerve work version bright ketchup wrestle erupt gun heart right stool earn pizza push";

const auto mnemonic_words = TurtleCoin::Utilities::str_split(mnemonic_phrase);

const auto [success, wallet_seed, timestamp] = Crypto::Mnemonics::decode(mnemonic_words);

if (success)
{
    std::cout << wallet_seed << std::endl;
}
```

## Deriving Wallet View Keys

With the seed in hand, it is now possible to generate the wallet view keys.

```c++
#include <crypto.h>

const auto [public_view, private_view] = Crypto::generate_wallet_view_keys(wallet_seed);
```

## Deriving Wallet Spend Keys

Wallets are generally composed of a primary subwallet and zero or more
additional subwallets.

As payment IDs are not supported, subwallets must be used to specify individual
accounts for services and other types of integrations.

The keys for each subwallet are derived from the seed and a subwallet index.

Using a subwallet index allows for *fast* restoration of subwallet keys in a
deterministic way.

To generate the primary (first) subwallet:

```c++
#include <crypto.h>

const auto [public_spend, private_spend] = Crypto::generate_wallet_spend_keys(wallet_seed, 0);
```

## Public Addresses

### Encoding

```c++
#include <address_encoding.h>

const auto public_address = TurtleCoin::Utilities::encode_address(public_spend, public_view);
```

### Decoding

```c++
#include <address_encoding.h>

const auto [success, public_spend, public_view] = TurtleCoin::Utilities::decode_address(public_address);
```

## Wallet Creation

Upon initialization or restoration of a wallet it is advisable to automatically
create a number of additional subwallets using a standard incrementing value.
This will help the user discover funds that may have been sent to a previously
used subwallet address without requiring the user to remember that a subwallet
was used or what index the subwallet was at. Furthermore, it enables advanced
functionality in that it becomes possible to automatically send transaction
change to a new subwallet address (if desired) in a deterministic way. In
addition, if funds are found for a subwallet while scanning, we then
automatically know to create additional subwallets for the next subwallet index.
This therefore makes it possible for the full recovery of all subwallets for the
given seed which further improves the user experience.

When doing this, given that we can underive the public key of a transaction, we
can then match an output of a transaction via a std::find on a vector (or map)
of public keys of the subwallets in a wallet very easily.

Full sample subwallet generation of 1,000 subwallets upon wallet creation:

```c++
#include <crypto.h>

const auto [wallet_seed, mnemonic_words, timestamp] = Crypto::generate_wallet_seed();

const auto [public_view, private_view] = Crypto::generate_wallet_view_keys(wallet_seed);

std::map<crypto_public_key_t, crypto_secret_key_t> subwallets;

std::map<crypto_public_key_t, std::string> subwallet_addresses;

for (size_t i = 0; i < 1000; ++i)
{
    const auto [public_spend, private_spend] = Crypto::generate_wallet_spend_keys(wallet_seed, i);

    subwallets.insert({public_spend, private_spend});
    
    const auto public_address = TurtleCoin::Utilities::encode_address(public_spend, public_view);
    
    subwallet_addresses.insert({public_spend, public_address});
}
```
