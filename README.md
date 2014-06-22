# iblt

An Invertible Bloom Lookup Table implemented in Ruby and C

## Description

This is an implementation for an Invertible Bloom Lookup Table (IBLT) as defined by Goodrich & Mitzenmacher. For a link to their paper on the subject, look in the Resources section.

An IBLT is a variation of a Bloom filter that includes the following methods:

- insert(key, value): Insert the key-value pair into the IBLT. This method is always successful if all keys are distinct and returns nil.
- delete(key, value): Remove the key-value pair from the IBLT. This method is always successful if the item exists in the IBLT and returns nil. No checking is done, so DO NOT delete an item unless it is definitely in the IBLT. 
- \[\](key): Returns the value for the given key or nil if not found. This method will sometimes return nil even though the item has been inserted, which is a property of IBLTs.
- inspect!(): Returns a string of all key-value pairs that have been inserted into the IBLT. This method will sometimes fail to return all key-value pairs that have been inserted, which is a property of IBLTs. This method also destroys the data.  

For this particular implementation, both the key and value must be integers. Also, a separate array is created for each hash function so that hashes yield distinct locations. Each array is of size m/k where m is the total size and k is the number of hash functions used. Instead of k distinct hash functions, we use k number of runs of CRC32 with a slightly different seed (incrementing the seed by 1).  

## Usage

Load the gem and create a new IBLT object:

```ruby
require 'iblt'
iblt = IBLT.new
```

You can define the total size of the IBLT, the number of hash functions, and the seed value, if you want (The defaults are :size => 100, :hashes => 4, :seed => Time.now.to_i):

```ruby
iblt = IBLT.new(:size => 1000, :hashes => 6, :seed => 12345)
```

Insert items into the IBLT:

```ruby
iblt.insert(1,1) # => nil
iblt.insert(2,2) # => nil
```

Get items from the IBLT:

```ruby
iblt[1] # => 1
iblt[2] # => 2
```

Delete an item from the IBLT and verify deletion:

```ruby
iblt.delete(1,1) # => nil
iblt[1] # => nil
```

List out contents of IBLT (WARNING: This destroys the data!):

```ruby
iblt.insert(3,3) # => nil
iblt.inspect! # => "{2=>2, 3=>3}"
iblt[2] # => nil
iblt[3] # => nil
iblt.inspect! # => "{}"
```

## Bugs

- There are occasional Segmentation Faults on insert

## TODO

- Accept strings as values for key-value pair instead of integers
- Implement non-destructive inspect method (by making copy of data to inspect)
- Implement as linked list instead of multidimensional array for better time performance
- Improve error handling
- Write RSpec tests

## Credits

- Aris Adamantiadis wrote the CRC32 C library that I'm using

## Resources

- Formal Definition of an IBLT: [Invertible Bloom Lookup Tables](http://arxiv.org/pdf/1101.2245.pdf)

## License

MIT License - Copyright (c) 2014 David Karhi
