/**
 * @file Provide some additional interactivity for the definitions chunk type.
 */

(function($) {

  'use strict';

  // Provide a client-side theme implementation for definitions chunks.
  Drupal.theme.prototype.chunk__definitions = function(configuration, fieldName, langcode, delta) {
    var chunkInstance, output, items, item, type;

    // Retrieve the chunk instance.
    chunkInstance = Drupal.chunks.fields[fieldName].chunks[delta].chunkInstance;

    output = '<dl class="chunk definitions-chunk">';

    items = configuration.definitions.split(/\n/);

    for (var i = 0; i < items.length; i++) {

      item = items[i];

      // Do not process empty items.
      if (item === '') {
        continue;
      }

      // Determine type.
      if (item[0] === '-') {
        type = 'dd';
        item = item.slice(1);
      }
      else {
        type = 'dt';
      }

      // If the format is plain text, run the text through chunkPlain
      if (Drupal.settings.chunks.fields[fieldName].instances[chunkInstance].settings.format === 'plain_text') {
        item = Drupal.checkPlain(item);
      }

      // Trim whitespace.
      item = item.replace(/^\s+|\s+$/g, '');

      // Do not display empty items.
      if (item === '') {
        continue;
      }

      output += '<' + type + '>' + item + '</' + type + '>';
    }

    output += '</dl>';

    return output;
  };

})(jQuery);
