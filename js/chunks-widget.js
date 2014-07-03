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
        var fieldName, chunksField, fieldSettings;

        fieldName = $(this).attr('field_name');
        fieldSettings = Drupal.settings.chunks[fieldName];

        // Create a new instance of ChunksField.
        chunksField = Drupal.chunks.fields[fieldName] = new ChunksField(this);

        // unset "empty" property. It's only purpose is to make sure the chunks
        // property is a dictionary instead of an array.
        delete fieldSettings.chunks.empty;

        // Prep any new staged chunks.
        $('tr.staged.ajax-loaded', e).once(function() {
          var chunksTable = Drupal.tableDrag[chunksField.classFieldName + '-values'];
          chunksTable.makeDraggable(this);
          if ($.cookie('Drupal.tableDrag.showWeight') != 1) {
            chunksTable.hideColumns();
          }
        });

        // We always have a staged chunk queued up after each request.
        fieldSettings.loadingStaged = false;
        if (typeof fieldSettings.queueNext !== 'undefined' && fieldSettings.queueNext !== false) {
          $('.ajax-progress.nothing-staged', chunksField.element).remove();
          chunksField.chunks[fieldSettings.queueNext].addButton.trigger('mousedown');
        }
        fieldSettings.queueNext = false;

      });
    }
  };
})(jQuery);
