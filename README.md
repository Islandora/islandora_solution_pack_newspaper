# Newspaper Solution Pack [![Build Status](https://travis-ci.org/Islandora/islandora_solution_pack_newspaper.png?branch=7.x)](https://travis-ci.org/Islandora/islandora_solution_pack_newspaper)

## Introduction

This module packages functions for ingesting and displaying newspaper content.

## Requirements

This module requires the following modules/libraries:

* [Islandora](https://github.com/islandora/islandora)
* [Tuque](https://github.com/islandora/tuque)
* [Islandora Paged Content](https://github.com/islandora_paged_content)
* [Islandora OCR](https://github.com/Islandora/islandora_ocr)
* [Islandora Large Image Solution Pack](https://github.com/Islandora/islandora_solution_pack_large_image)
* [Islandora Openseadragon](https://github.com/islandora_openseadragon) (optional)

## Installation

Install as usual, see [this](https://drupal.org/documentation/install/modules-themes/modules-7) for further information.

## Configuration

Configuration options are found at Administration » Islandora » Solution pack configuration » Newspapers (admin/islandora/solution_pack_config/newspaper).

Select configuration options for which issue and page derivatives are created locally.
Select the **Parent Solr Field** which holds the parent issue PID for a newspaper page.

Select the **Use Solr** option to switch from using the resource index to using Solr to generate the issue list for a newspaper.
This also reveals three required Solr fields for this option.
 * Newspaper issue parent field
 * Issued date field
 * Sequence field

**Note**: The above three fields are for Solr records of newspaper **issues** in your repository. 

![Derivative and solr configuration options](https://user-images.githubusercontent.com/2738244/40319734-293d1fbe-5cf7-11e8-906a-04ce3cbc3986.png)

Also select a viewer for the newspaper issue view and page view.

![Issue view and page view configuration options](https://user-images.githubusercontent.com/2857697/33495870-bcc7f2ae-d68d-11e7-834d-cade66be45b4.jpg)

## Documentation
:warning: <br/>Deleting a newspaper object directly (Manage > Properties >  Delete Newspaper) will delete all its child Issue objects, and their associated Page objects. Highlighted in red in this diagram shows all that will be deleted if the newspaper Locusta Newspaper is deleted. 
+![newspaper_diagram](https://user-images.githubusercontent.com/2738244/30652457-6ea939e0-9df6-11e7-851b-d298ca1e631b.png)

However, deleting a newspaper via its parent collection (by deleting the collection or by using "Delete members of this collection") will cause that newspaper's Issue objects to be orphaned.

Further documentation for this module is available at [our wiki](https://wiki.duraspace.org/display/ISLANDORA/Newspaper+Solution+Pack).

## Troubleshooting/Issues

Having problems or solved a problem? Check out the Islandora google groups for a solution.

* [Islandora Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora)
* [Islandora Dev Group](https://groups.google.com/forum/?hl=en&fromgroups#!forum/islandora-dev)

## Maintainers/Sponsors

Current maintainers:

* [Jared Whiklo](https://github.com/whikloj)

## Development

If you would like to contribute to this module, please check out [CONTRIBUTING.md](CONTRIBUTING.md). In addition, we have helpful [Documentation for Developers](https://github.com/Islandora/islandora/wiki#wiki-documentation-for-developers) info, as well as our [Developers](http://islandora.ca/developers) section on the [Islandora.ca](http://islandora.ca) site.

## License

[GPLv3](http://www.gnu.org/licenses/gpl-3.0.txt)
