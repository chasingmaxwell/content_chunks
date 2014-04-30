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
        var fieldName, classFieldName, langcode, chunks, setActiveChunk, showStagedChunk;

        fieldName = $(this).attr('field_name');
        classFieldName = $(this).attr('field_name').replace(/_/g, '-');
        langcode = $(this).attr('langcode');
        chunks = $('.chunk-wrapper', $(this));

        // Helper function to set "active" class on a specific chunk.
        setActiveChunk = function(chunk) {
          chunks.removeClass('active');
          chunk.addClass('active');
        };

        // Helper function to show a staged chunk.
        showStagedChunk = function(prevSibling) {
          var stagedRow, stagedChunk, stagedDelta, stagedViewElement;
          stagedRow = $('tr.staged', context);
          stagedChunk = $('.chunk-wrapper', stagedRow);
          stagedDelta = stagedChunk.attr('delta');
          stagedViewElement = $(':input[name="' + fieldName + '[' + langcode + '][' + stagedDelta + '][view]"]');
          stagedViewElement.val('type_selection');
          stagedViewElement.trigger('change');
          stagedRow.insertAfter(prevSibling);
          stagedRow.show();
          // @TODO: reset weights.
          // @TODO: reset odd/even classes.
        };

        chunks.each(function(index, element) {
          var delta, classPrepend, viewElement, view, active, addButton;

          delta = $(element).attr('delta');
          viewElement = $(':input[name="' + fieldName + '[' + langcode + '][' + delta + '][view]"]');
          view = viewElement.val();
          classPrepend = '.' + classFieldName + '-' + delta + '-';
          active = $(element).hasClass('active');
          addButton = $(':input[name="' + fieldName + '-' + delta + '-add-after"]');

          // If this chunk is staged, hide it.
          if (view == 'staged') {
            $(element).parents('#' + fieldName + '-' + delta + '-chunk-row').hide();
          }

          // Switch to configuration view when errors are detected.
          if ($(classPrepend + 'configuration .error', element).length > 0) {
            viewElement.val('configuration');
            viewElement.trigger('change');
          }

          // Switch to configuration view upon type selection.
          $(':input[name="' + fieldName + '[' + langcode + '][' + delta + '][type]"]', element).bind('keyup.chunkTypeSelected click.chunkTypeSelected', function(e) {
            if (e.type === 'click' || e.type === 'keyup' && e.charCode === '13') {
              viewElement.val('configuration');
              viewElement.trigger('change');
              // Set active class on the last chunk with user interaction.
              setActiveChunk($(element));
            }
          });

          // Switch to preview view when "Preview" button is pressed.
          $(classPrepend + 'preview-button', element).bind('keyup.chunkPreview mousedown.chunkPreview', function(e) {
            if (e.type === 'click' || e.type === 'keyup' && e.charCode === '13') {
              viewElement.val('preview');
              viewElement.trigger('change');
              // Set active class on the last chunk with user interaction.
              setActiveChunk($(element));
              addButton.focus();
              // Remove active state on button.
              removeActiveState(this);
            }
          });

          // Switch to configuration view when "Edit" button is pressed.
          $(classPrepend + 'edit-button', element).bind('keyup.chunkEdit mousedown.chunkEdit', function(e) {
            if (e.type === 'mousedown' || e.type === 'keyup' && e.charCode === '13') {
              viewElement.val('configuration');
              viewElement.trigger('change');
              // Set active class on the last chunk with user interaction.
              setActiveChunk($(element));
              // Remove active state on button.
              removeActiveState(this);
            }
          });

          // Swith to preview view with "Cancel" button is pressed.
          $(classPrepend + 'cancel-button', element).bind('keyup.chunkEdit mousedown.chunkEditCancel', function(e) {
            if (e.type === 'mousedown' || e.type === 'keyup' && e.charCode === '13') {
              viewElement.val('preview');
              viewElement.trigger('change');
              // Set active class on the last chunk with user interaction.
              setActiveChunk($(element));
              // Remove active state on button.
              removeActiveState(this);
            }
          });

          // Switch to removed view when "Remove" button is pressed.
          $(classPrepend + 'remove-button', element).bind('mousedown.chunkEdit', function(e) {
            viewElement.val('removed');
            viewElement.trigger('change');
            $('#' + fieldName + '-' + delta + '-chunk-row').hide();
            // @TODO: reset odd/even classes.
          });

          // Switch to type_selection view for staged chunk when add after
          // button is pressed.
          $(classPrepend + 'add-after-button', element).bind('mousedown.chunkAdd', function(e) {
            showStagedChunk('#' + fieldName + '-' + delta + '-chunk-row');
          });

          // Always set focus to the active chunk's button.
          if (active) {
            addButton.focus();
          }

        });

        // Switch to type_selection view for staged chunk when add before
        // button is pressed.
        $(':input[name="' + fieldName + '-add-before"]', context).bind('mousedown.chunkAdd', function(e) {
          showStagedChunk('#' + fieldName + '-chunks-field .add-chunk-action-before-row');
        });

      });
    }
  };

})(jQuery);

