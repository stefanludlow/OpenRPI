#!/usr/bin/ruby
# Script to take list of crafts which is in creation order and sort it by major craft then by subcraft
# The purpose of this is to make the admin listing of 'crafts' that shows all crafts easier to read
# Author: Grommit
# Usage: ordercrafts.rb crafts_file_path
# e.g. in the code in boot_crafts, it calls ordercrafts.rb ../regions/crafts
# In the in-game code usage, it calls it only on the bp (swapping then carries over the sorted crafts to pp)
# Therefore install this file in bp/lib for proper use

# Open crafts
lines = IO.readlines(ARGV[0])

# Save and then remove the first and last lines which are header/footer stuff
firstline = lines[0]
lastline = lines[-1]

lines.delete_at(0)
lines.delete_at(-1)

# Slice the lines into arrays of lines each starting with a line that starts with ^craft
sliced_array = lines.slice_before(/^craft/).collect { |craft| craft}

# Sort by first line, which is craft [always same] then craft family then subcraft [always same] then subcraft name. Which is perfect.
sorted_crafts = sliced_array.sort { |a,b| a[0] <=> b[0]}

# Open crafts file for writing
File.open(ARGV[0],'w') do |s|
  # Write out sorted crafts -- with header and footer
  s.puts firstline
  sorted_crafts.each { |craft|
    craft.each { |l| s.puts l }
  }
  s.puts lastline
end

# Notify of task completion
puts "#{sorted_crafts.count} crafts sorted."
