/**
 * @file Provide a client-side theme implementation for list chunks.
 */

(function() {
  Drupal.theme.prototype.chunk__list = function(configuration) {
    var list, output, list_item;

    // Break configuration.list into an array.
    list = configuration.list.split(/\n/);

    output = '<' + configuration.style + ' class="chunk list-chunk">';

    for (var key in list) {

      // If the format is plain text, run the text through chunkPlain
      if (Drupal.settings.chunks.list.instance_type_settings.format === 'plain_text') {
        list_item = Drupal.checkPlain(list[key]);
      }
      // Otherwise don't do anything at all.
      else {
        list_item = list[key];
      }

      // Do not display empty strings.
      if (list_item === '') {
        continue;
      }

      // Add the list item to output.
      output += '<li>' + list_item + '</li>';
    }

    output += '</' + configuration.style + '>';

    return output;
  };
})();
