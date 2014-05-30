/**
 * @file Provide a client-side theme implementation for text chunks.
 */

(function($) {
  Drupal.theme.prototype.chunk__p = function(config) {
    var output = '<p class="chunk p-chunk">';
    output += Drupal.checkPlain(config.p);
    output += '</p>';
    return output;
  };
})(jQuery);