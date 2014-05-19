/**
 * @file Provide a client-side theme implementation for text chunks.
 */

(function() {
  Drupal.theme.prototype.chunk__text = function(config) {
    var output = '<div class="chunk text-chunk">';
    output += Drupal.checkPlain(config.text);
    output += '</div>';
    return output;
  };
})();
