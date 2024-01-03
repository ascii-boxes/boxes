#
# boxes - Command line filter to draw/remove ASCII boxes around text
# Copyright (c) 1999-2024 Thomas Jensen and the boxes contributors
#
# This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
# License, version 3, as published by the Free Software Foundation.
# This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
# warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
# You should have received a copy of the GNU General Public License along with this program.
# If not, see <https://www.gnu.org/licenses/>.
#

source "https://rubygems.org"
# Hello! This is where you manage which Jekyll version is used to run.
# When you want to use a different version, change it below, save the
# file and run `bundle install`. Run Jekyll with `bundle exec`, like so:
#
#     bundle exec jekyll serve
#
# This will help ensure the proper Jekyll version is running.
# Happy Jekylling!

gem "rouge", "~> 3.26.0"

# If you want to use GitHub Pages, remove the "gem "jekyll"" above and
# uncomment the line below. To upgrade, run `bundle update github-pages`.
gem "github-pages", group: :jekyll_plugins

# If you have any plugins, put them here!
group :jekyll_plugins do
    #gem 'jekyll-last-modified-at', "~> 1.3.0"  # https://github.com/gjtorikian/jekyll-last-modified-at
    gem 'jekyll-paginate', '~> 1.1.0'          # https://jekyllrb.com/docs/pagination/
    gem 'jekyll-redirect-from', '~> 0.16.0'    # https://github.com/jekyll/jekyll-redirect-from
    gem 'jekyll-seo-tag', "~> 2.7.1"           # http://jekyll.github.io/jekyll-seo-tag/
    gem "jekyll-sitemap", "~> 1.4.0"           # https://github.com/jekyll/jekyll-sitemap
    gem "jemoji", "~> 0.12.0"
end

# Windows and JRuby does not include zoneinfo files, so bundle the tzinfo-data gem and associated library.
platforms :mingw, :x64_mingw, :mswin, :jruby do
    gem "tzinfo", "~> 1.2"
    gem "tzinfo-data"
end

# Performance-booster for watching directories on Windows
gem "wdm", "~> 0.1.1", :platforms => [:mingw, :x64_mingw, :mswin]
