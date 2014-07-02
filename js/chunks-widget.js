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
        var fieldName, chunksField;

        fieldName = $(this).attr('field_name');

        // Create a new instance of ChunksField.
        chunksField = Drupal.chunks.fields[fieldName] = new ChunksField(this);

        // Prep any new staged chunks.
        $('tr.staged.ajax-loaded', e).once(function() {
          var chunksTable = Drupal.tableDrag[chunksField.classFieldName + '-values'];
          chunksTable.makeDraggable(this);
          chunksTable.hideColumns();
        });

      });
    }
  };
})(jQuery);
