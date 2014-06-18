/**
 * @file
 *   JS behavior for Entity Reference chunk type.
 */

(function($) {

  'use strict';

  // Provide a client-side theme implementation for text chunks.
  Drupal.theme.prototype.chunk__entity_reference = function(config, fieldName, langcode, delta) {
    return '<p>' + config.reference + '</p>';
  };

})(jQuery);
