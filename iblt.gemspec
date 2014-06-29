Gem::Specification.new do |s|
  s.name = "iblt"
  s.version = "0.2.0"
  
  s.authors = ["David Karhi"]
  s.email = 'dkarhi@gmail.com'
  s.description = "An Invertible Bloom Lookup Table implemented in Ruby and C"
  s.summary = "Invertible Bloom Lookup Table in Ruby and C"
  s.homepage = "http://github.com/dkarhi/iblt"
  s.license = 'MIT'
  s.date = "2014-06-28"
  s.files = `git ls-files`.split("\n")
  s.extensions = ["ext/ciblt/extconf.rb"]

  s.add_development_dependency 'rspec'
  s.add_development_dependency "rake"
  s.add_development_dependency "rake-compiler"
end
