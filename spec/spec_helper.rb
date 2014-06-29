require 'bundler/setup'
Bundler.setup

require 'iblt'

RSpec.configure do |config|
  config.color         = true
  config.formatter     = 'documentation'
end
