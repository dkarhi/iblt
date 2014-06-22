class IBLT
 
  def initialize(opts = {})
    @opts = {
      :size    => 100,
      :hashes  => 4,
      :seed    => Time.now.to_i,
    }.merge(opts)

    @iblt = CIBLT.new(@opts[:size], @opts[:hashes], @opts[:seed]))
  end

  # This method inserts a key-value pair into the IBLT. This operation always
  # succeeds, assuming that all keys are distinct.
  def insert(key, value)
    @iblt.insert(key, value)
  end

  # This method deletes a key-value pair from the IBLT. This operation always
  # succeeds if the key-value pair is present.
  def delete(key, value)
    @iblt.delete(key, value)
  end

  # This method returns the value that is associated with the key it is passed.
  # If the key does not exist, then nil is returned. There is a low probability
  # that this operation may return nil even though the key exists.
  def [](key)
    @iblt.[](key)
  end

  # This methods returns all key-value pairs as a string
  def inspect()
    @iblt.inspect()
  end
  alias :to_s :inspect

end
