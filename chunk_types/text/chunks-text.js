(function($) {
  Drupal.theme.prototype.text_chunk = function(config) {
    return Drupal.checkPlain(config.text);
  };
})(jQuery);
