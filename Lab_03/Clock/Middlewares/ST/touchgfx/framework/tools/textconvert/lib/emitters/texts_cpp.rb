# Copyright (c) 2018(-2025) STMicroelectronics.
# All rights reserved.
#
# This file is part of the TouchGFX 4.25.0 distribution.
#
# This software is licensed under terms that can be found in the LICENSE file in
# the root directory of this software component.
# If no LICENSE file comes with this software, it is provided AS-IS.
#
###############################################################################/
require 'lib/string_collector'

class TextsCpp < Template
  def initialize(characters, text_entries, typographies, languages, output_directory, remap_global, generate_binary_translations, copy_translations_to_ram)
    @characters = characters # one array (per language, or all at index 0 if global remapping) of the needed strings in optimal order
    @remap_global = remap_global
    @generate_binary_translations = generate_binary_translations
    @copy_translations_to_ram = copy_translations_to_ram
    super(text_entries, typographies, languages, output_directory)
    @cache = {}
  end
  def cache_file
    File.join(@output_directory, 'cache/TextsCpp.cache')
  end
  def input_path
    File.join(root_dir,'Templates','Texts.cpp.temp')
  end
  def output_path
    'src/Texts.cpp'
  end
  def output_filename
    File.join(@output_directory, output_path)
  end
  def run
    #build @cache and compare with file if exists
    @cache["remap"] = @remap_global
    @cache["languages"] = get_capitalized_languages
    if @remap_global=="yes"
      @cache["characters"] = @characters.flatten
    end
    @cache["copy_translations_to_ram"] = @copy_translations_to_ram
    if @copy_translations_to_ram=="yes"
      lengths = {}
      @languages.each_with_index do |l, i|
        lengths[l] = language_length(i)
      end
      @cache["lengths"] = lengths
    end

    new_cache_file = false
    if not File::exists?(cache_file)
      new_cache_file = true
    else
      #cache file exists, compare data with cache file
      old_cache = JSON.parse(File.read(cache_file))
      new_cache_file = (old_cache != @cache)
    end

    if new_cache_file
      #write new cache file
      FileIO.write_file_silent(cache_file, @cache.to_json)
    end

    if !File::exists?(output_filename) || new_cache_file
      #generate TypedTextDatabase.cpp
      super
    end
  end

  def remap_global?
    @remap_global=="yes"
  end

  def generate_binary_files?
    @generate_binary_translations=="yes"
  end

  def get_capitalized_languages
    @languages.map(&:capitalize)
  end

  def get_number_of_languages
    @languages.count
  end

  def has_languages?
    !@languages.empty?
  end

  def is_rtl?
    text_entries.is_rtl
  end

  def get_substrings_and_offsets
    unicode_array_to_hex_offset_comment(@characters.flatten)
  end

  def language_length(l)
    @characters[l].length
  end

  def get_max_language_length
    @characters.inject(0){|m,c| c.length > m ? c.length : m }
  end

  def copy_translations?
    @copy_translations_to_ram=="yes"
  end

  def get_pragma
    @pragma ||= @copy_translations_to_ram=="yes" ? "" : "TEXT_LOCATION_FLASH_PRAGMA\r\n"
  end

  def get_attribute
    @attribute ||= @copy_translations_to_ram=="yes" ? "" : " TEXT_LOCATION_FLASH_ATTRIBUTE"
  end
end
