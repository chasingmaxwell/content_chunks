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

  // Provide helper for determining whether there is a Drupal.ajax managed ajax
  // request in progress.
  Drupal.ajaxInProgress = function() {
    for (var id in Drupal.ajax) {
      if (Drupal.ajax[id].ajaxing === true) {
        return true;
      }
    }
    return false;
  };

  // Initialize all behavior necessary for chunks.
  Drupal.behaviors.chunksWidget = {
    attach: function(context, settings) {

      $('.chunks-field', context).each(function(i, e) {
        var fieldName, fieldSettings, chunksField;

        fieldName = $(this).attr('field_name');
        fieldSettings = Drupal.settings.chunks.fields[fieldName];

        // Create a new ChunksField if we haven't already.
        if (typeof Drupal.chunks.fields[fieldName] === 'undefined') {
          // Create a new instance of ChunksField.
          chunksField = Drupal.chunks.fields[fieldName] = new ChunksField(this);

          // unset "empty" property. It's only purpose is to make sure the chunks
          // property is a dictionary instead of an array.
          delete fieldSettings.chunks.empty;

          fieldSettings.queueNext = false;
        }
        // If we already have a ChunksField, perform update operations.
        else {

          chunksField = Drupal.chunks.fields[fieldName];

          // Update the ChunksField with the new state.
          chunksField.setProperties(this);
          chunksField.retrieveChunks();

          // Run the next queued action when ajax stops.
          $(document).bind('ajaxStop.runNextQueuedAction', function() {

            // Only do this once.
            $(document).unbind('ajaxStop.runNextQueuedAction');

            setTimeout(function() {
              chunksField.actions.runNext();
            }, 0);
          });
        }
      });

      // Register when a preview has been loaded via ajax.
      if (typeof settings.chunks !== 'undefined' && typeof settings.chunks.previewLoaded !== 'undefined') {
        var fieldName, delta;
        fieldName = settings.chunks.previewLoaded.fieldName;
        delta = settings.chunks.previewLoaded.delta;
        Drupal.chunks.fields[fieldName].chunks[delta].previewLoading = false;
        delete settings.chunks.previewLoaded;
      }
    }
  };
})(jQuery);
