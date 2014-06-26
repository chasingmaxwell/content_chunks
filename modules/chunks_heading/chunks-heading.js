/**
 * @file Provide a client-side theme implementation for heading chunks.
 */

(function() {
  Drupal.theme.prototype.chunk__heading = function(configuration) {
    var output = '';

    output += '<h' + configuration.level + ' class="chunk heading-chunk"';
    if (configuration.id !== '') {
      output += ' id="' + Drupal.checkPlain(configuration.id) + '"';
    }
    output += '>' + Drupal.checkPlain(configuration.text) + '</h' + configuration.level + '>';

    return output;
  };
})();
