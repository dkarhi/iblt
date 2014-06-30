require 'spec_helper'

describe "IBLT" do
  let(:iblt) { IBLT.new }

  describe "#insert" do
    it 'returns nil on success' do
      expect(iblt.insert("key","value")).to eq(nil)
    end
  end

  describe "#delete" do
    it 'returns nil on success' do
      iblt.insert("key","value") 
      expect(iblt.delete("key","value")).to eq(nil)
    end
  end

  describe "#[]" do
    it 'returns nil if key not found' do
      expect(iblt["key"]).to eq(nil)
    end

    it 'returns value if key exists' do
      iblt.insert("key","value")
      expect(iblt["key"]).to eq("value")
    end
  end

  describe "#inspect!" do
    it 'returns empty square brackets if no items exist' do
      expect(iblt.inspect!).to eq("{}")
    end

    it 'returns an item if it has been inserted' do
      iblt.insert("key","value")
      expect(iblt.inspect!).to eq("{key=>value}")
    end
  end
end
