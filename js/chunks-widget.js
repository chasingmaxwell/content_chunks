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
        }
        // If we already have a ChunksField, perform update operations.
        else {

          chunksField = Drupal.chunks.fields[fieldName];

          // Update the ChunksField with the new state.
          chunksField.setProperties(this);
          chunksField.retrieveChunks();

          // Prep any new staged chunks.
          $('tr.staged.ajax-loaded', e).once(function() {
            var chunksTable = Drupal.tableDrag[chunksField.classFieldName + '-values'];
            chunksTable.makeDraggable(this);
            if ($.cookie('Drupal.tableDrag.showWeight') != 1) {
              chunksTable.hideColumns();
            }
          });

          // Do we need to load another staged chunk?
          fieldSettings.loadingStaged = false;
          if (typeof fieldSettings.queueNext !== 'undefined' && fieldSettings.queueNext !== false) {
            $('.ajax-progress.nothing-staged', chunksField.element).remove();
            if (fieldSettings.queueNext > 0) {
              chunksField.chunks[fieldSettings.queueNext - 1].addButton.trigger({type: 'mousedown', which: 1});
            }
            else {
              $(':input[name="' + fieldName  + '-add-before"]', chunksField.element).trigger({type: 'mousedown', which: 1});
            }
          }
          fieldSettings.queueNext = false;
        }
      });

      // Register when a preview has been loaded via ajax.
      if (typeof settings.chunks.previewLoaded !== 'undefined') {
        var fieldName, delta;
        fieldName = settings.chunks.previewLoaded.fieldName;
        delta = settings.chunks.previewLoaded.delta;
        Drupal.chunks.fields[fieldName].chunks[delta].previewLoading = false;
        delete settings.chunks.previewLoaded;
      }
    }
  };
})(jQuery);
