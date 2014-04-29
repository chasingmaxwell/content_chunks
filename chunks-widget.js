/**
 * @file
 * Add some extra reactivity to the chunks field widget.
 */

(function($){

  'use strict';

  // Remove active state from element by briefly removing it from the DOM.
  var removeActiveState = function(el) {
    var p, s;
    p = el.parentNode;
    s = el.nextSibling;

    p.removeChild(el);
    p.insertBefore(el, s);
  };

  Drupal.behaviors.chunksWidget = {
    attach: function(context, settings) {
      // Do some stuff
      $('.chunks-field', context).each(function() {
        var fieldName, classFieldName, langcode, chunks, setActiveChunk;

        fieldName = $(this).attr('field_name');
        classFieldName = $(this).attr('field_name').replace(/_/g, '-');
        langcode = $(this).attr('langcode');
        chunks = $('.chunk-wrapper', $(this));

        // Helper function to set "active" class on a specific chunk.
        setActiveChunk = function(chunk) {
          chunks.removeClass('active');
          chunk.addClass('active');
        };

        chunks.each(function(delta, element) {
          var classPrepend, viewElement, view, active, addButton;

          classPrepend = '.' + classFieldName + '-' + delta + '-';
          viewElement = $(':input[name="' + fieldName + '[' + langcode + '][' + delta + '][view]"]');
          view = viewElement.val();
          active = $(element).hasClass('active');
          addButton = $(':input[name="' + fieldName + '-' + delta + '-add-after"]');

          // Switch to configuration view when errors are detected.
          if ($(classPrepend + 'configuration .error', element).length > 0) {
            viewElement.val('configuration');
            viewElement.trigger('change');
          }

          // Switch to configuration view upon type selection.
          $(':input[name="' + fieldName + '[' + langcode + '][' + delta + '][type]"]', element).bind('click.chunkTypeSelected', function(e) {
            viewElement.val('configuration');
            viewElement.trigger('change');
            // Set active class on the last chunk with user interaction.
            setActiveChunk($(element));
          });

          // Switch to preview view when "Preview" button is selected.
          $(classPrepend + 'preview-button', element).bind('mousedown.chunkPreview', function(e) {
            viewElement.val('preview');
            viewElement.trigger('change');
            // Set active class on the last chunk with user interaction.
            setActiveChunk($(element));
            addButton.focus();
          });

          // Switch to configuration view when "Edit" button is selected.
          $(classPrepend + 'edit-button', element).bind('mousedown.chunkEdit', function(e) {
            viewElement.val('configuration');
            viewElement.trigger('change');
            // Set active class on the last chunk with user interaction.
            setActiveChunk($(element));
          });

          // Always set focus to the active chunk's button.
          if (active) {
            addButton.focus();
          }
        });

      });
    }
  };

})(jQuery);

