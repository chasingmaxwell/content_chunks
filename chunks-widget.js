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
        var field, fieldName, classFieldName, langcode, chunks, setActiveChunk, showStagedChunk, resetStripes, config, saveConfig, restoreConfig;

        field = $(this);
        fieldName = field.attr('field_name');
        classFieldName = field.attr('field_name').replace(/_/g, '-');
        langcode = field.attr('langcode');
        chunks = $('.chunk-wrapper', field);
        config = {};

        // Helper function to set "active" class on a specific chunk.
        setActiveChunk = function(chunk) {
          chunks.removeClass('active');
          chunk.addClass('active');
        };

        // Helper function to show a staged chunk.
        showStagedChunk = function(prevSibling) {
          var stagedRow, stagedChunk, stagedDelta, stagedViewElement;
          stagedRow = $('tr.staged', field);
          stagedChunk = $('.chunk-wrapper', stagedRow);
          stagedDelta = stagedChunk.attr('delta');
          stagedViewElement = $(':input[name="' + fieldName + '[' + langcode + '][' + stagedDelta + '][view]"]');
          stagedViewElement.val('type_selection');
          stagedViewElement.trigger('change');
          stagedRow.removeClass('staged');
          stagedRow.insertAfter(prevSibling);
          stagedRow.show();
          // @TODO: reset weights.
          resetStripes();
        };

        // Helper function to reset odd/even striping on visible chunk rows.
        resetStripes = function() {
          var visibleChunks = $('.chunk-wrapper:visible', field);
          visibleChunks.each(function(i, element) {
            var delta, parentRow;

            delta = $(element).attr('delta');
            parentRow = $('#' + fieldName + '-' + delta + '-chunk-row');

            if (((i + 1) % 2) == 1) {
              parentRow.removeClass('even').addClass('odd');
            }
            else {
              parentRow.removeClass('odd').addClass('even');
            }
          });
        };

        // Helper function to save configuration.
        saveConfig = function(delta) {
          config[delta] = {};

          $('[name^="' + fieldName + '[' + langcode + '][' + delta + '][configuration]"]').each(function(i, element) {
            var name = $(element).attr('name');
            config[delta][name] = $(element).val();
          });
        };

        // Helper function to restore configuration.
        restoreConfig = function(delta) {
          $('[name^="' + fieldName + '[' + langcode + '][' + delta + '][configuration]"]').each(function(i, element) {
            var name = $(element).attr('name');
             $(element).val(config[delta][name]);
          });

          delete config[delta];
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
            if (e.type === 'click' || e.type === 'keyup' && e.keyCode === 13) {
              this.checked = true;
              $(this).trigger('change');
              viewElement.val('configuration');
              viewElement.trigger('change');
              // Set active class on the last chunk with user interaction.
              setActiveChunk($(element));
              // Add focus to first form item.
              $('[name^="' + fieldName + '[' + langcode + '][' + delta + '][configuration]"]').first().focus();
            }
          });

          // Switch to preview view when "Preview" button is pressed.
          $(classPrepend + 'preview-button', element).bind('keyup.chunkPreview mousedown.chunkPreview', function(e) {
            if (e.type === 'mousedown' || e.type === 'keyup' && e.keyCode === 13) {
              viewElement.val('preview');
              viewElement.trigger('change');
              // Set active class on the last chunk with user interaction.
              setActiveChunk($(element));
              addButton.focus();
            }
          });

          // Switch to configuration view when "Edit" button is pressed.
          $(classPrepend + 'edit-button', element).bind('keyup.chunkEdit mousedown.chunkEdit', function(e) {
            if (e.type === 'mousedown' || e.type === 'keyup' && e.keyCode === 13) {
              viewElement.val('configuration');
              viewElement.trigger('change');
              // Set active class on the last chunk with user interaction.
              setActiveChunk($(element));
              // Remove active state on button.
              removeActiveState(this);
              // Save configuration so we can restore it if we cancel.
              saveConfig(delta);
            }
          });

          // Swith to preview view with "Cancel" button is pressed.
          $(classPrepend + 'cancel-button', element).bind('keyup.chunkEditCancel mousedown.chunkEditCancel', function(e) {
            if (e.type === 'mousedown' || e.type === 'keyup' && e.keyCode === 13) {
              viewElement.val('preview');
              viewElement.trigger('change');
              // Set active class on the last chunk with user interaction.
              setActiveChunk($(element));
              // Remove active state on button.
              removeActiveState(this);
              // Restore configuration.
              restoreConfig(delta);
            }
          });

          // Switch to removed view when "Remove" button is pressed.
          $(classPrepend + 'remove-button', element).bind('keyup.chunkRemove mousedown.chunkRemove', function(e) {
            if (e.type === 'mousedown' || e.type === 'keyup' && e.keyCode === 13) {
              e.preventDefault();
              viewElement.val('removed');
              viewElement.trigger('change');
              $('#' + fieldName + '-' + delta + '-chunk-row').hide();
              resetStripes();
            }
          });

          // Switch to type_selection view for staged chunk when add after
          // button is pressed.
          $(classPrepend + 'add-after-button', element).bind('keyup.chunkAdd mousedown.chunkAdd', function(e) {
            if (e.type === 'mousedown' || e.type === 'keyup' && e.keyCode === 13) {
              showStagedChunk('#' + fieldName + '-' + delta + '-chunk-row');
            }
          });

          // Always set focus to the active chunk's button.
          if (active) {
            addButton.focus();
          }

        });

        // Switch to type_selection view for staged chunk when add before
        // button is pressed.
        $(':input[name="' + fieldName + '-add-before"]', field).bind('keyup.chunkAdd mousedown.chunkAdd', function(e) {
          if (e.type === 'mousedown' || e.type === 'keyup' && e.keyCode === 13) {
            showStagedChunk('#' + fieldName + '-chunks-field .add-chunk-action-before-row');
          }
        });

      });
    }
  };

})(jQuery);

