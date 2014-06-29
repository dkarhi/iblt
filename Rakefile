require 'bundler/gem_tasks'
require 'rake'
require 'rspec'
require 'rspec/core/rake_task'
require 'rake/extensiontask'

Bundler::GemHelper.install_tasks
Rake::ExtensionTask.new('ciblt')
RSpec::Core::RakeTask.new(:spec)
Rake::Task[:spec].prerequisites << :clean
Rake::Task[:spec].prerequisites << :compile
