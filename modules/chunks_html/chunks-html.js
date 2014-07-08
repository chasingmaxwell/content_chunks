/**
 * @file Provide a client-side theme implementation for full HTML chunks.
 */

(function() {
  Drupal.theme.prototype.chunk__html = function(configuration) {
    return '<div class="chunk html-chunk">' + configuration.html + '</div>';
  };
})();
