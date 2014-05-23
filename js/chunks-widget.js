/**
 * @file
 * Add some extra reactivity to the chunks field widget.
 */

(function($){

  'use strict';

  // Add the Drupal.chunks namespace.
  Drupal.chunks = {
    fields: {}
  };

  // Initialize all behavior necessary for chunks.
  Drupal.behaviors.chunksWidget = {
    attach: function(context, settings) {

      // Create a new ChunksField object for each chunks field.
      $('.chunks-field', context).once('initializeChunkField', function(i, e) {
        var fieldName;

        fieldName = $(this).attr('field_name');

        // Create a new instance of ChunksField.
        Drupal.chunks.fields[fieldName] = new ChunksField(this);

      });
    }
  };
})(jQuery);
