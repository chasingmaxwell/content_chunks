/**
 * @file Provide a client-side theme implementation for text chunks.
 */

(function() {
  Drupal.theme.prototype.chunk__p = function(config) {
    var output = '<div class="chunk p-chunk">';
    output += Drupal.checkPlain(config.p);
    output += '</div>';
    return output;
  };
})();