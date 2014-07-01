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
      $('.chunks-field', context).each(function(i, e) {
        var fieldName;

        fieldName = $(this).attr('field_name');

        // Create a new instance of ChunksField.
        Drupal.chunks.fields[fieldName] = new ChunksField(this);

      });
    },
    detach: function(context, settings) {
      $('.chunks-field', context).each(function(i, e) {
        var fieldName, chunksField;

        fieldName = $(this).attr('field_name');
        chunksField = Drupal.chunks.fields[fieldName];

        // Unbind all event handlers.
        chunksField.destroyEventHandlers();
      });
    }
  };
})(jQuery);
